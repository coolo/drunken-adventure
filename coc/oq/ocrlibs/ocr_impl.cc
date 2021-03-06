
#include <stdio.h>
#include <iostream>
#include <exception>
#include <cerrno>
#include <dirent.h>
#include <sys/time.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <vector>
#include <list>
#include <algorithm>    // std::min

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <opencv2/imgproc/imgproc.hpp>

#include "../ppmclibs/tinycv.h"
#include "ocr.h"

#define DEBUG 0
#define DEBUG2 0

#define VERY_DIFF 0.0
#define VERY_SIM 1000000.0

using namespace cv;
using namespace std;

void processColorsWithPalette(Mat& m, const Mat &palette)
{
  m.convertTo(m, CV_8U);

  Mat lab;
  Mat lab_palette;
  cvtColor(palette, lab_palette, CV_BGR2Lab);
  cvtColor(m, lab, CV_BGR2Lab);

  lab.convertTo(lab, CV_32F);
  lab_palette.convertTo(lab_palette, CV_32F);

  for (int y = 0; y < m.rows; y++) {
    for (int x = 0; x < m.cols; x++) {
      Vec3f f = lab.at<Vec3f>(y, x);
      double min_distance = INFINITY;
      int best_match = 0;
      for (int p = 0; p < lab_palette.cols; ++p) {
	Vec3f pc = lab_palette.at<Vec3f>(0, p);
	double distance = norm(f - pc);
	if (distance < min_distance) {
	  min_distance = distance;
	  best_match = p;
	}
      }
      m.at<Vec3b>(y, x) = palette.at<Vec3b>(0, best_match);
    }
  }
}

// default palette for OCR
void processColors(Mat &m)
{
  Vec3b white = Vec3b(255, 255, 255);
  Vec3b nick_color = Vec3b(143, 188, 191);
  
  cv::Mat palette(1, 12, CV_8UC3);
  palette.at<Vec3b>(0, 0) = Vec3b(0, 0, 0);
  palette.at<Vec3b>(0, 1) = white;
  palette.at<Vec3b>(0, 2) = Vec3b(64, 64, 64);
  palette.at<Vec3b>(0, 3) = Vec3b(100, 100, 100);
  palette.at<Vec3b>(0, 9) = Vec3b(140, 140, 140);
  palette.at<Vec3b>(0, 10) = Vec3b(180, 180, 140);
  palette.at<Vec3b>(0, 11) = Vec3b(215, 215, 215);
  // be aware it's BGR
  palette.at<Vec3b>(0, 4) = Vec3b(17, 188, 120);
  palette.at<Vec3b>(0, 5) = nick_color;
  palette.at<Vec3b>(0, 6) = Vec3b(44, 208, 230);
  palette.at<Vec3b>(0, 7) = Vec3b(0, 0, 255);
  palette.at<Vec3b>(0, 8) = Vec3b(176, 104, 16);

  processColorsWithPalette(m, palette);

  for (int y = 0; y < m.rows; y++) {
    for (int x = 0; x < m.cols; x++) {
      Vec3b nc = m.at<Vec3b>(y, x);
      if (nc == nick_color) {
	m.at<Vec3b>(y, x) = white;
      }
    }
  }
}

struct Image {
  Mat img;
};

int find_next_gray_row(const Mat &m, int y)
{
  Vec3b gray(100, 100, 100);
  while (y < m.rows) {
    bool all_gray = true; 
    for (int x = 2; x < 10; x++) {
      //printf("%02x%02x%02x ", m.at<cv::Vec3b>(y, x)[2], m.at<cv::Vec3b>(y, x)[1], m.at<cv::Vec3b>(y, x)[0]);
      if (m.at<cv::Vec3b>(y, x) != gray) {
	all_gray = false;
	break;
      }
    }
    if (all_gray)
      return y;
    else
      y++;
  }
  return y;
}

void find_green(const Mat &m, int &y, int &x)
{
  Vec3b green(17, 188, 120);
  for (; y < m.rows - 31; y++) {
    for (x = 10; x < m.cols; x++) {
      if (m.at<cv::Vec3b>(y, x) == green) {
	return;
      }
    }
  }
  y = -1;
  return;
}

Mat reduce_text(const Mat &m)
{
  Mat nick = m;
  Vec3b donation = Vec3b(0xb0, 0x68, 0x10);
  for (int y = 0; y < m.rows; y++) {
    if (m.at<Vec3b>(y, 273) == donation) {
      nick = Mat(nick, Rect(0, 0, m.cols, y - 5));
      break;
    }
  }
  cvtColor(nick, nick, CV_BGR2GRAY);
  nick.convertTo(nick, CV_8UC1);
  int non_zeros[nick.rows];
  int last_non_zero = 0;
  for (int y = 0; y < nick.rows; y++) {
    non_zeros[y] = 0;
    for (int x = 0; x < nick.cols; x++) {
      uchar value = (nick.at<uchar>(y, x) - 64) / 36;
      nick.at<uchar>(y, x) = value;
      if (nick.at<uchar>(y, x) != 0)
	non_zeros[y]++;
    }
    if (non_zeros[y])
      last_non_zero = y;
  }
  //imwrite("nick.png", nick);
  if (last_non_zero + 3 >= nick.rows )
    return nick;
  return Mat(nick, Rect(0, 0, nick.cols, last_non_zero + 3));
}

char map_ocr_to_print(char p) {
  if (p >= 5)
    return '@';
  if (p == 4)
    return '*';
  if (p == 3)
    return '+';
  if (p == 2)
    return '-';
  if (p == 1)
    return '.';
  return ' ';
}

char map_print_to_ocr(char p) {
  if (p == '@')
    return 5;
  if (p == '*')
    return 4;
  if (p == '+')
    return 3;
  if (p == '-')
    return 2;
  if (p == '.')
    return 1;
  return 0;
}

void display_mat(FILE *f, const Mat &m)
{
  int start_x = 0;
  for (int x = 0; !start_x && x < m.cols; x++) {
    for (int y = 0; y < m.rows; y++) {
      unsigned char p = m.at<uchar>(y, x);
      if (p > 2) {
	start_x = x;
	break;
      }
    }
  }

  for (int index = 0; index < int(m.cols / 100 + 1); ++index) {
    for (int y = 0; y < m.rows; y++) {
      for (int x = start_x + index * 100; x < m.cols; x++) {
	unsigned char p = m.at<uchar>(y, x);
	fprintf(f, "%c", map_ocr_to_print(p));
	if (x > start_x + index * 100 + 150) {
	  break;
	}
      }
      fprintf(f, "\n");
    }
  }
}

class OCRLetter {
public:
  string letter;
  OCRLetter() { _matrix = 0; _width = 0; _height = 0; }
  OCRLetter(const string &_l, const vector<string> &lines);
  OCRLetter &operator=(const OCRLetter&);
  OCRLetter(const OCRLetter&);
  ~OCRLetter();
  void print() const; 

  int width() const { return _width; }
  int height() const { return _height; }

  void set(int y, int x, char s) { _matrix[y][x] = s; }
  char at(int y, int x) const { return _matrix[y][x]; }
  
private:

  unsigned int _width, _height;
  char **_matrix;
};

struct FoundLetter {
  FoundLetter(const OCRLetter &_l, int _y, int _x, double _mse) {
    letter = _l;
    x = _x;
    y = _y;
    mse = _mse;
  }
  FoundLetter() {
    x = y = 0;
    mse = 0;
  }
  OCRLetter letter;
  int x;
  int y;
  double mse;
};

OCRLetter::~OCRLetter()
{
  for (unsigned int i = 0; i < _height; ++i)
    delete [] _matrix[i];
  delete [] _matrix;
}

OCRLetter &OCRLetter::operator=(const OCRLetter &other)
{
  if (this != &other) // protect against invalid self-assignment
    {
      for (unsigned int i = 0; i < _height; ++i) {
	delete [] _matrix[i];
      }
      delete [] _matrix;
 
      letter = other.letter;
      _width = other._width;
      _height = other._height;
      
      _matrix = new char*[_height];
      for (unsigned int i = 0; i < _height; ++i) {
	_matrix[i] = new char[_width];
	std::copy(other._matrix[i], other._matrix[i] + _width, _matrix[i]);
      }
    }
  // by convention, always return *this
  return *this;
}

OCRLetter::OCRLetter(const OCRLetter &other)
{
  letter = other.letter;
  _width = other._width;
  _height = other._height;
  
  // 1: allocate new memory and copy the elements
  _matrix = new char*[_height];
  for (unsigned int i = 0; i < _height; ++i) {
    _matrix[i] = new char[_width];
    std::copy(other._matrix[i], other._matrix[i] + _width, _matrix[i]);
  }
}

OCRLetter::OCRLetter(const string &_l, const vector<string> &lines)
{
  this->letter = _l;
  _width = 0;
  _height = lines.size();
  for (vector<string>::const_iterator it = lines.begin(); it != lines.end(); ++it) {
    if (_width < it->length())
      _width = it->length();
  }
  _matrix = new char*[_height];
  int index = 0;
  vector<string>::const_iterator it = lines.begin();
  for (; it != lines.end(); ++it, ++index) {
    _matrix[index] = new char[_width];
    for (unsigned int i = 0; i < _width; ++i) {
      char value = ' ';
      if (i < it->length())
	value = it->at(i);
      _matrix[index][i] = map_print_to_ocr(value);
    }
  }
}

void OCRLetter::print() const {
  for (unsigned int y = 0; y < _height; ++y) {
    for (unsigned int x = 0; x < _width; ++x) {
      cout << map_ocr_to_print(_matrix[y][x]);
    }
    cout << endl;
  }
}

vector<OCRLetter> read_letters(const char *fn)
{
  vector<OCRLetter> letters;
  
  FILE *f = fopen(fn, "r");
  if (!f) {
    perror("opening chars file");
    return letters;
  }
  char buffer[100];
  string reading_char;
  vector<string> lines;
    
  while (fgets(buffer, sizeof(buffer) - 1, f)) {
    int len = strlen(buffer);
    while (len && (buffer[len-1] == ' ' || buffer[len-1] == '\n')) {
      buffer[--len] = 0;
    }
    if (!reading_char.length()) {
      reading_char = buffer;
      continue;
    }
    if (len == 2 && !strcmp(buffer, "==")) {
      letters.push_back(OCRLetter(reading_char, lines));
      reading_char.resize(0);
      lines.clear();
      continue;
    }
    lines.push_back(string(buffer));
  }
  if (reading_char.length())
    letters.push_back(OCRLetter(reading_char, lines));
  fclose(f);

  return letters;
}

double max_value(const OCRLetter &letter)
{
  double max_values = 0;
  for (int y = 0; y < letter.height(); y++) 
    for (int x = 0; x < letter.width(); x++) 
      max_values += letter.at(y, x) * 36;
  return max_values / ( letter.height() * letter.width());
}

double check_letter_position(const OCRLetter &letter, const Mat &m, int start_y, int start_x)
{
  double mismatches = 0;
  for (int y = 0; y < letter.height(); y++) {
    for (int x = 0; x < letter.width(); x++) {
      int ist = m.at<uchar>(start_y + y, start_x + x) * 36;
      int soll = letter.at(y, x) * 36;
      mismatches += (ist - soll) * (ist - soll);
    }
  }
  double mse = mismatches / ( letter.height() * letter.width());
  mse /= 256;
  return mse;
}

void copy_letter(const FoundLetter &letter, Mat &newm)
{
  int height = letter.letter.height();
  int width = letter.letter.width();
  for (int y1 = 0; y1 < height; ++y1)
    for (int x1 = 0; x1 < width; ++x1)
      newm.at<uchar>(letter.y + y1, letter.x + x1) = letter.letter.at(y1, x1) * 36 + 64;
}

void remove_letter(const FoundLetter &letter, Mat &m)
{
  int height = letter.letter.height();
  int width = letter.letter.width();
  for (int y1 = 0; y1 < height; ++y1)
    for (int x1 = 0; x1 < width; ++x1)
      if (letter.letter.at(y1, x1))
	m.at<uchar>(letter.y + y1, letter.x + x1) = 0;
}
		   
void find_letter(vector<OCRLetter> &letters, Mat &m, list<FoundLetter> &findings, float match_limit = 12) {
  FoundLetter last;
   
   for (int y = 0; y < m.rows; y++) {
    for (int x = 0; x < m.cols; x++) {
      vector<OCRLetter>::iterator best_letter = letters.end();
      double best_mm = INFINITY;
 
      vector<OCRLetter>::iterator it = letters.begin();
      for (; it != letters.end(); ++it) {
	if (y + it->height() >= m.rows)
	  continue;
	if (x + it->width() >= m.cols)
	  continue;
	double mismatches = check_letter_position(*it, m, y, x);
	if (best_mm > mismatches && mismatches < match_limit) {
	  best_mm = mismatches;
	  best_letter = it;
	}
	if (mismatches < 1) { // *very* sure here
	  break;
	}
      }
      if (best_letter != letters.end()) {
	last = FoundLetter(*best_letter, y, x, best_mm);
	findings.push_back(last);
      }
    }
  }
}

bool compare_position (const FoundLetter& first, const FoundLetter& second) {
  if (second.y > first.y + 15)
    return true;
  if (first.y > second.y + 15)
    return false;
  return second.x > first.x;
}

bool compare_mse (const FoundLetter& first, const FoundLetter& second) {
  double mse1 = first.mse;
  double mse2 = second.mse;
  if (fabs(mse1 - mse2) > 0.5)
    return mse2 > mse1;
  int pixels1 = first.letter.height() * first.letter.width();
  int pixels2 = second.letter.height() * second.letter.width();
  if (abs(pixels1 - pixels2) > 20)
    return pixels2 < pixels1;
  return (mse2 > mse1);
}

void save_letters(const vector<OCRLetter> &letters, const char *filename) {
  FILE *f = fopen(filename, "w");
  vector<OCRLetter>::const_iterator it = letters.begin();
  it = letters.begin();
  for (; it != letters.end(); ++it) {
    fprintf(f, "%s\n", it->letter.c_str());
    for (int y = 0; y < it->height(); ++y) {
      for (int x = 0; x < it->width(); ++x) {
	fprintf(f, "%c", map_ocr_to_print(it->at(y, x)));
      }
      fprintf(f, "\n");
    }
    fprintf(f, "==\n");
  }
  fclose(f);
  printf("saved %s\n", filename);
}

string map_letters(vector<OCRLetter> &letters, Mat &m, float match_limit = 12) {
  list<FoundLetter> findings;
  vector<OCRLetter>::iterator it = letters.begin();
  if (getenv("DISPLAYNEW"))
    display_mat(stdout, m);
  find_letter(letters, m, findings, match_limit);
  list<FoundLetter> confirmed;
  
  findings.sort(compare_mse);
  int counter = 0;
  (void)counter;
  Mat found(m.size(), CV_8UC1);
  found = Scalar(0);
#if 0
  list<FoundLetter>::iterator lit2 = findings.begin();
  for (; lit2 != findings.end(); ++lit2)
    cout << lit2->letter.letter << " " << lit2->y << " " << lit2->x << " " << lit2->mse  << endl;
#endif
  while (!findings.empty()) {
    FoundLetter best = *findings.begin();
    if (best.mse > match_limit)
      break;
    confirmed.push_back(best);
    findings.pop_front();
    //cout << best.letter.letter << " " << best.y << " " << best.x << " " << best.mse << endl;
#if 0
    copy_letter(best, found);
    char buffer[100];
    sprintf(buffer, "founds/found-%04d.png", counter++);
    imwrite(buffer, found);
#endif
    remove_letter(best, m);

    list<FoundLetter>::iterator lit2 = findings.begin();
    for (; lit2 != findings.end(); ++lit2)
      lit2->mse = check_letter_position(lit2->letter, m, lit2->y, lit2->x);
    findings.sort(compare_mse);
    //display_mat(stdout, m);
  }

  confirmed.sort(compare_position);

  list<FoundLetter>::const_iterator lit = confirmed.begin();
  string result;
  int offset = 0;
  for (; lit != confirmed.end(); ++lit) {
    int delta = lit->x - offset;
    while (delta > 2) {
      result.append(" ");
      delta -= 5;
    }
    result.append(lit->letter.letter);
    offset = lit->x + lit->letter.width() + 1;
  }

  return result;
}

void image_chat_ocr(Image *s)
{
  int counter = 0;
  cv::Mat m = s->img;

  vector<OCRLetter> nick_letters = read_letters("chars_nick");
  vector<OCRLetter> text_letters = read_letters("chars_text");

  processColors(m);
  //imwrite("reduced.png", m);
  int y = find_next_gray_row(m, 5) + 1;
  for (;;) {
    int x;
    find_green(m, y, x);
    if (y < 0 || y > m.rows - 31)
      break;
    // we found that green
    Mat nick = reduce_text(Mat(m, Range(y+1, y+31), Range(0,x - 10)));
    string l = map_letters(nick_letters, nick);
#if 0
    save_letters(nick_letters, "chars_nick.new");
    unlink("chars_nick");
    rename("chars_nick.new", "chars_nick");
#endif
    
    int next_y = find_next_gray_row(m, y + 30);
    if (next_y > m.rows - 31)
      break;
    Mat text = reduce_text(Mat(m, Range(y + 32, next_y), Range(0, m.cols)));
    char buffer[100] = "";
#if 0
    sprintf(buffer, "text-%d.png", counter);
    imwrite(buffer, text);
#endif
    string tl = map_letters(text_letters, text);
#if 0
    save_letters(text_letters, "chars_text.new");
    unlink("chars_text");
    rename("chars_text.new", "chars_text");
#endif
    y = next_y + 1;

    cout << l << ": " << tl << " " << buffer << endl;
    counter++;
  }
}

int image_troop_count(Image *s, const char *fn) {
  cv::Mat m = s->img;

  vector<OCRLetter> letters = read_letters(fn);

  processColors(m);

  cvtColor(m, m, CV_BGR2GRAY);
  m.convertTo(m, CV_8UC1);
  for (int y = 0; y < m.rows; y++) {
    for (int x = 0; x < m.cols; x++) {
      uchar value = m.at<uchar>(y, x) / 51;
      m.at<uchar>(y, x) = value;
    }
  }
  //display_mat(stdout, m);
  string tl = map_letters(letters, m);
  //cout << "TL " << tl << endl;
  tl.erase(remove(tl.begin(), tl.end(), ' '), tl.end());
  if (tl.length() && tl[0] == 'x')
    tl.erase(0, 1);
  return atoi(tl.c_str());
}

int image_base_count(Image *s, const char *fn) {
  cv::Mat m = s->img;

  vector<OCRLetter> letters = read_letters(fn);

  processColors(m);

  cvtColor(m, m, CV_BGR2GRAY);
  m.convertTo(m, CV_8UC1);
  for (int y = 0; y < m.rows; y++) {
    for (int x = 0; x < m.cols; x++) {
      uchar value = m.at<uchar>(y, x) / 51;
      m.at<uchar>(y, x) = value;
    }
  }
  //display_mat(stdout, m);
  string tl = map_letters(letters, m);
  tl.erase(remove(tl.begin(), tl.end(), ' '), tl.end());
  if (tl.length() && tl[0] == 'x')
    tl.erase(0, 1);
  //cout << "TL " << tl << " " << atol(tl.c_str()) << endl;
  return atol(tl.c_str());
}

static double dX = .749;

double as[4];
double bs[4];
double cs[4];
int ds[4];

bool out_of_box(int y, int x)
{
  if (y < 0 || y > 640)
    return true;
  if (y < as[0] * x + cs[0] + ds[0] * bs[0])
    return true;
  if (y < as[1] * x + cs[1] + ds[1] * bs[1])
    return true;
  if (y > as[2] * x + cs[2] + ds[2] * bs[2])
    return true;
  if (y > as[3] * x + cs[3] + ds[3] * bs[3])
    return true;
  
  return false;
}

int green_distance(Mat &reduced, const Mat &hsv, const Vec3b &green, int index)
{
  double a = as[index];
  double b = bs[index];
  double c = cs[index];
  bool first_green = true;
  int last_min = 0;
  int last_green = 0;

  for (int i = 0; i < 500; i++) {

    int min = 555;
    vector<uchar> values;
    int greens = 0;

    for (int x = 0; x < reduced.cols; x++) {

      int y = a * x + c + i * b;

      if (out_of_box(y, x))
	continue;
      if (reduced.at<Vec3b>(y, x) == green)
	greens++;
      // calculate sliding average
      values.push_back(hsv.at<Vec3b>(y, x)[0]);
      if (values.size() > 10) {
	values.erase(values.begin());
	float sum = 0;
	vector<uchar>::const_iterator it = values.begin();
	for (; it != values.end(); ++it)
	  sum += *it;
	uchar nv = sum / 10;
	if (min > nv)
	  min = nv;
      }
    }
    if (min < 23)
      return i;
    //cout << index << " " << i << " " << greens << " " << min << endl;
    const int green_limit = 280;
    if (greens <= green_limit && !first_green && float(last_min) / min > 1.2) 
      return i;
    
    if (greens > green_limit) {
      first_green = false;
      last_green = i;
    }
    last_min = min;
  }
  return last_green;
}

Point cross_between(int index1, int index2)
{
  Point t;
  double x = (cs[index2] + ds[index2] * bs[index2] -
	      cs[index1] - ds[index1] * bs[index1]) / (as[index1] - as[index2]);
  t.x = int(x);
  t.y = int(as[index1] * x + cs[index1] + ds[index1] * bs[index1]);
  return t;
}

std::vector<int> image_find_red_line(Image *s)
{
  Mat m, hsv;

  cvtColor(s->img, hsv, CV_BGR2HSV);
  s->img.convertTo(m, CV_8UC3);

  //Mat chan[3];
  //split(hsv, chan);
  //imwrite("hsv.png", chan[0]);
  cv::Mat palette(1, 3, CV_8UC3);
  Vec3b green = Vec3b(73, 186, 166);
  palette.at<Vec3b>(0, 0) = green;
  palette.at<Vec3b>(0, 1) =  Vec3b(59, 152, 179);
  palette.at<Vec3b>(0, 2) =  Vec3b(72, 180, 185);

  Mat reduced = m.clone();
  processColorsWithPalette(reduced, palette);

  as[0] = -dX;
  cs[0] = 564 * dX;
  bs[0] = 1;
  ds[0] = 0;
  
  as[1] = dX;
  cs[1] = - 799 * dX;
  bs[1] = 1;
  ds[1] = 0;
    
  as[2] = -dX;
  cs[2] = 564 * dX + 851;
  bs[2] = -1;
  ds[2] = 0;
      
  as[3] = dX;
  cs[3] = 44 * 19.34 - 799 * dX;
  bs[3] = -1;
  ds[3] = 0;
  
  ds[0] = green_distance(reduced, hsv, green, 0);
  ds[1] = green_distance(reduced, hsv, green, 1);
  ds[2] = green_distance(reduced, hsv, green, 2);
  ds[3] = green_distance(reduced, hsv, green, 3);
  
  //imwrite("reduced.png", reduced);

  Point last_green_p1, last_green_p2;
#if 1
  last_green_p1 = cross_between(2, 3);;
  last_green_p2 = cross_between(2, 1);;
  
  float dX1 = (float(last_green_p2.x) - last_green_p1.x) / (float(last_green_p2.y) - last_green_p1.y);
  for (int x = last_green_p1.x; x < last_green_p2.x; x++) {
    int y = (x - last_green_p1.x) / dX1 + last_green_p1.y;
    if (y < 0)
      continue;
    m.at<Vec3b>(y, x) = Vec3b(255, 0, 0);
  }

  last_green_p1 = cross_between(3, 0);;
  last_green_p2 = cross_between(3, 2);;
  
  float dX2 = (float(last_green_p2.x) - last_green_p1.x) / (float(last_green_p2.y) - last_green_p1.y);
  for (int x = last_green_p1.x; x < last_green_p2.x; x++) {
    int y = (x - last_green_p1.x) / dX2 + last_green_p1.y;
    if (y < 0)
      continue;
    m.at<Vec3b>(y, x) = Vec3b(255, 0, 0);
  }

  last_green_p1 = cross_between(1, 0);;
  last_green_p2 = cross_between(1, 2);;

  float dX3 = (float(last_green_p2.x) - last_green_p1.x) / (float(last_green_p2.y) - last_green_p1.y);
  for (int x = last_green_p1.x; x < last_green_p2.x; x++) {
    int y = (x - last_green_p1.x) / dX3 + last_green_p1.y;
    if (y < 0)
      continue;
    m.at<Vec3b>(y, x) = Vec3b(255, 0, 0);
  }

  last_green_p1 = cross_between(0, 3);
  last_green_p2 = cross_between(0, 1);
  
  dX3 = (float(last_green_p2.x) - last_green_p1.x) / (float(last_green_p2.y) - last_green_p1.y);
  for (int x = last_green_p1.x; x < last_green_p2.x; x++) {
    int y = (x - last_green_p1.x) / dX3 + last_green_p1.y;
    if (y < 0)
      continue;
    m.at<Vec3b>(y, x) = Vec3b(255, 0, 0);
  }

  imwrite("line.png", m);
#endif
  
  vector<int> res;
  last_green_p1 = cross_between(0, 3);
  last_green_p2 = cross_between(0, 1);
  res.push_back(last_green_p1.x);
  res.push_back(last_green_p1.y);
  res.push_back(last_green_p2.x);
  res.push_back(last_green_p2.y);
  last_green_p1 = cross_between(2, 3);
  last_green_p2 = cross_between(2, 1);
  res.push_back(last_green_p1.x);
  res.push_back(last_green_p1.y);
  res.push_back(last_green_p2.x);
  res.push_back(last_green_p2.y);

  return res;
}

float check_area(const Mat &m, const Mat &obj, int x, int y, int times, int &tx, int &ty)
{
  Mat result;
  int x1 = max(0, (x)*4);
  int x2 = min(m.cols, (x+times)*4 - 2 + obj.cols);
  int y1 = max(0, (y)*4);
  int y2 = min(m.rows, (y+1)*4 - 2 + obj.rows);
  if (getenv("DEBUG")) {
    printf("check_area %dx%d - %dx%d\n", y1,x1, y2,x2);
  }
  Mat roi = Mat(m, Range(y1, y2), Range(x1, x2));
  matchTemplate(roi, obj, result, CV_TM_SQDIFF_NORMED);

  /// Localizing the best match with minMaxLoc
  double minVal; double maxVal; Point minLoc; Point maxLoc;
  Point matchLoc;

  minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
  tx = x1 + minLoc.x;
  ty = y1 + minLoc.y;
  return minVal;
}

double getPSNR(const Mat& I1, const Mat& I2)
{
  Mat s1;
  absdiff(I1, I2, s1);       // |I1 - I2|
  s1.convertTo(s1, CV_32F);  // cannot make a square on 8 bits
  s1 = s1.mul(s1);           // |I1 - I2|^2

  Scalar s = sum(s1);        // sum elements per channel

  double sse = s.val[0] + s.val[1] + s.val[2]; // sum channels

  double mse = sse / (double)(I1.channels() * I1.total());
  if (!mse) {
    return VERY_SIM;
  }
  return 10.0 * log10((255 * 255) / mse);
}

float detect_mse(Image *s, const char *filename)
{
  Mat scene = s->img, scene_small, obj_small;
  resize(scene, scene_small, Size(scene.cols / 4, scene.rows / 4));
  
  Mat obj = imread(filename, CV_LOAD_IMAGE_COLOR);
  resize(Mat(obj, Range(0, obj.rows / 4 * 4), Range(0, obj.cols / 4 * 4)), obj_small, Size(obj.cols / 4, obj.rows / 4));
  Mat result;
  matchTemplate(scene_small, obj_small, result, CV_TM_SQDIFF_NORMED);

  float min = 10;
  int bx = 0, by = 0;
  float min_result = 10;
  for (int y = 0; y < result.rows; y++) {
    for (int x = 0; x < result.cols; x++) {
      const float limit = 0.13;
      float p = result.at<float>(y, x);
      if (p < min_result) {
	min_result = p;
      }
      if (p > limit)
	continue;
      int tx, ty;
      int times = 1;
      while (x < result.cols && result.at<float>(y, x + times) <= limit) {
	times++;
      }
      float t = check_area(scene, obj, x, y, times, tx, ty);
      x += times - 1;
      if (t < min) {
	min = t;
	bx = tx;
	by = ty;
      }
    }
  }

  float psnr = getPSNR(obj, Mat(scene, Range(by, by + obj.rows), Range(bx, bx + obj.cols)));
  if (getenv("DEBUG")) {
    cout << "min " << min << " " << bx << " " << by << " " << min_result << " " << psnr << endl;
  }
  return psnr;
}

int find_places(Mat &img, const Mat &obj)
{
  Mat result;

  matchTemplate(img, obj, result, CV_TM_SQDIFF_NORMED);
  
  int strong_buildings = 0;
  while (1) {

    /// Localizing the best match with minMaxLoc
    double minVal; double maxVal; Point minLoc; Point maxLoc;
    Point matchLoc;
    
    minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
    //cout << "weak_base " << minVal <<  " " << minLoc << endl;
    if (minVal > 0.06)
      break;
    for (int y = minLoc.y; y <= minLoc.y + obj.rows; y++)
      for (int x = minLoc.x; x <= minLoc.x + obj.cols; x++)
	result.at<float>(y, x) = 1;
    strong_buildings++;
  }

  return strong_buildings;
}

int weak_base(Image *s)
{
  Mat img = s->img;
  //cout << "ws\n";
  int ws = find_places(img, imread("ths/WS.png"));
  //cout << "as\n";
  int as = find_places(img, imread("ths/AS.png"));
  //cout << "cs\n";
  int cs = find_places(img, imread("ths/CS.png"));

  int ms = find_places(img, imread("ths/MS.png"));
  return ws * 4 + as * 3 + cs * 2 + ms;
}

typedef map<string, int> thmap;
std::vector<int> image_find_townhall(Image *s)
{
  thmap ths;
 
  //ths["ths/01-th-9.png"] = 9;
  ths["ths/02-th-8.png"] = 8;
  //ths["ths/03-th-10.png"] = 10;
  //ths["ths/04-th-11.png"] = 11;
  ths["ths/05-th-7.png"] = 7;
  //ths["ths/06-th-9.png"] = 9;

  vector<int> res;
  float best_mse = 0;
  res.push_back(15);
  res.push_back(0);

  for (thmap::const_iterator it = ths.begin(); it != ths.end(); ++it) {
    float mse = detect_mse(s, it->first.c_str());
    if (true || getenv("DEBUG")) {
      cout << it->first << " " << mse << " " << it->second << endl;
    }
    if (mse > best_mse) {
      best_mse = mse;
      if (mse > 17) {
	res.clear();
	res.push_back(it->second);
	int weak = 0;
	if (it->second == 8) {
	  weak = weak_base(s);
	}
	res.push_back(weak);
	return res;
      }
    }
  }
  return res;
}
