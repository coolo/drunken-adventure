
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

void processColors(Mat& m)
{
  m.convertTo(m, CV_8U);

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
      for (int p = 0; p < palette.cols; ++p) {
	Vec3f pc = lab_palette.at<Vec3f>(0, p);
	double distance = norm(f - pc);
	if (distance < min_distance) {
	  min_distance = distance;
	  best_match = p;
	}
      }
      Vec3b nc = palette.at<Vec3b>(0, best_match);
      if (nc == nick_color)
	nc = white;
      m.at<Vec3b>(y, x) = nc;
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
		   
void find_letter(vector<OCRLetter> &letters, Mat &m, list<FoundLetter> &findings) {
   float match_limit = 12;

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

string map_letters(vector<OCRLetter> &letters, Mat &m) {
  list<FoundLetter> findings;
  vector<OCRLetter>::iterator it = letters.begin();
  if (getenv("DISPLAYNEW"))
    display_mat(stdout, m);
  find_letter(letters, m, findings);
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
    if (best.letter.letter == ",") {
      //display_mat(stdout, m);
    }
    if (best.mse > 15 || best.mse * 3 > max_value(best.letter))
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
  while (tl.length() && tl[0] == ' ')
    tl.erase(0, 1);
  if (tl.length() && tl[0] == 'x')
    tl.erase(0, 1);
  return atoi(tl.c_str());
}
