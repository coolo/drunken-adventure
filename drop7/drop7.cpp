#include "opencv2/opencv.hpp"
#include "Field.h"
#include <iostream>
#include <limits>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <fann.h>

using namespace cv;
using namespace std;

void turn(Field &f, char t)
{
  double min = std::numeric_limits<double>::max();
  int col = -1;
  for (int x = 0; x < 7; x++) {
    if (f.at(0, x) != ' ')
      continue;
    
    Field droped = f.drop(t, x + 1);
    //cerr << droped.to_string();
    //cerr << endl << endl;

    double r = droped.recursive_rating(4);
    cerr << x << " " << r << endl;
    if (min > r) {
      col = x + 1;
      min = r;
    }
  }
  cerr << "best " << col << endl;
  f = f.drop(t, col);
  f.finalize_random();
  while (f.blink()) {
    f.finalize_random();
  }

  //cerr << f.to_string();
}

Mat screencap() {
  struct timeval tv;
  FILE *adb = popen("adb shell screencap -p", "r");
  gettimeofday(&tv, 0);
  char fname[300];
  sprintf(fname, "current-%ld.%ld.png", tv.tv_sec, tv.tv_usec);
  FILE *png = fopen(fname, "w");
  char buffer[3000];
  size_t all = 0;
  for (;;) {
    size_t bytes = fread(buffer, 1, sizeof(buffer) - 1, adb);
    all += bytes;
    if (!bytes)
      break;
    // now make sure we read 0x0d0a completey
    if (buffer[bytes-1] == 0xd) {
      buffer[bytes++] = fgetc(adb);
    }
    // undo EOL replacement
    for (unsigned int i = 0; i < bytes; ++i) {
      if (buffer[i] == 0xd && buffer[i+1] == 0xa ) {
	buffer[i] = 0xa;
	for (unsigned int j = i+1; j < bytes - 1; ++j)
	  buffer[j] = buffer[j+1];
	bytes--;
      }
    }
    fwrite(buffer, 1, bytes, png);
  }
  fclose(adb);
  fclose(png);
  if (all)
    rename(fname, "current.png");
  return imread("current.png");
}

Mat resized_screencap() {
  Mat resized;
  Mat frame = screencap();
  float scale = 200. / frame.cols;
  resize(frame, resized, Size(0, 0), scale, scale);

  resized = Mat(resized, Rect(0, 0, 192, 320));
  frame = resized;
  cvtColor(frame, frame, CV_BGR2GRAY );
  frame.convertTo(frame, CV_8UC1);
  imwrite("frame.png", frame);
  return frame;
}


/* the purpose of this function is to calculate the error between two images
  (scene area and object) ignoring slight colour changes */
double enhancedMSE(const Mat& I1, const Mat& I2) {
  assert(I1.channels() == 1);
  assert(I2.channels() == 1);
  assert(I1.rows == I2.rows);
  assert(I1.cols == I2.cols);
  
  double sse = 0;
  
  for (int j = 0; j < I1.rows; j++)
    {
      // get the address of row j
      const uchar* I1_data = I1.ptr<const uchar>(j);
      const uchar* I2_data = I2.ptr<const uchar>(j);
       
      for (int i = 0; i < I1.cols; i++)
        {
	  double diff = I1_data[i] - I2_data[i];
	  sse += diff * diff;
        }
    }
  
  double total = I1.total();
  return sse / total;
}


struct Button {
  char letter;
  Mat object;
  Mat eightbit;
  
  int cols, rows;
  int x, y;
  
  Button() {
    x = y = 0;
    letter = ' ';
  }
  
  Button(char _letter, const char *filename) {
    object = imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
    if (!object.cols) {
	    fprintf(stderr, "can't find %s\n", filename);
	    exit(13);
    }
    cols = object.cols;
    rows = object.rows;
    object.convertTo(eightbit, CV_8UC1);
    letter = _letter;
  }
};

vector<Button> buttons;

const int part_size = 20;
const int min_x = 13;
const int min_y = 79;

char find_letter(const Mat &frame, int x, int y)
{
  std::vector<Button>::const_iterator it = buttons.begin();
  Mat part(frame, Rect(x, y, part_size, part_size ));
  /* char buffer[300];
     sprintf(buffer, "part-%dx%d.png", y, x);
     imwrite(buffer, part); */
  
  double maxmse = HUGE_VAL;
  char maxletter = ' ';
  for (; it != buttons.end(); ++it) {
    double mse = enhancedMSE(part, it->eightbit );
    if (maxmse > mse) {
      maxmse = mse;
      maxletter = it->letter;
    }
  }
  //cerr << y << " " << x << " " << maxletter << " " << maxmse << endl;
  if (maxmse < -700) {
   
  }

  int limit = 750;
  if (maxletter == 'B')
    limit = 200;
  
  if (maxletter == 'A') {
    char buffer[300];
    sprintf(buffer, "part-%03dx%03d-%05.1lf-%c.png", y, x, maxmse, maxletter);
    imwrite(buffer, part);
    limit = 300;
  }
  
  if (maxmse < limit) {
    return maxletter;
  } else {
    /*    char buffer[300];
    sprintf(buffer, "part-%03dx%03d-%lf-%c.png", y, x, maxmse, maxletter);
    imwrite(buffer, part); */
  }
  return ' ';
}

Field scan_field(char &found) {
  Field f;
  
  Mat frame = resized_screencap();

  const int xcords[7] = { 15, 40, 65, 90, 115, 140, 165 };
  const int ycords[7] = { 77, 104, 132, 159, 187, 214, 242 };
  
  for (int y = 0; y < 7; y++) {
    for (int x = 0; x < 7; x++) {
      char maxletter = find_letter(frame, xcords[x], ycords[y] );
      
      if (maxletter != ' ')
	f.set(y, x, maxletter);
    }
  }
  found = find_letter(frame, 90, 45);
  return f;
}

int main()
{
#if 0
    const char*  init =
      "       \n"
      "       \n"
      "    67 \n"
      "    617\n"
      "    1A2\n"
      "666ABAA\n"
      "BABAABB\n";
    
    Field t = Field::from_string(init);
    cerr << t.drop('1', 5).to_string() << " " << t.drop('1', 6).to_string() << endl;
    cerr << t.drop('1', 5).rating() << " " << t.drop('1', 6).rating() << endl;
    cerr << t.drop('1', 5).recursive_rating(4) << " " << t.drop('1', 6).recursive_rating(4) << endl;
    exit(0);
#endif
    
  buttons.push_back(Button('1', "1.png"));
  buttons.push_back(Button('2', "2.png"));
  buttons.push_back(Button('3', "3.png"));
  buttons.push_back(Button('4', "4.png"));
  buttons.push_back(Button('5', "5.png"));
  buttons.push_back(Button('6', "6.png"));
  buttons.push_back(Button('7', "7.png"));
  buttons.push_back(Button('A', "A.png"));
  buttons.push_back(Button('B', "B.png"));

  struct fann *ann;
  ann = fann_create_from_file("drop7-GOOD.net");

  while (1) {
    char found;
    Field f = scan_field(found);
    
    cerr << f.to_string() << found << endl;

    double max = std::numeric_limits<double>::min();
    int col = -1;
    for (int x = 0; x < 7; x++) {
      if (f.at(0, x) != ' ')
	continue;
    
      Field droped = f.drop(found, x + 1);
      //cerr << droped.to_string();
      //cerr << endl << endl;
      
      float inputs[49*9];
      droped.ann_input(inputs);
      //cout << "NEW\n" << n.to_string();
            
      float *calc = fann_run(ann, inputs);
      cout << "calced " << x + 1 << " " << calc[0] << endl;
    
      if (max < calc[0]) {
	col = x + 1;
	max = calc[0];
      }
    }
    cerr << "best " << col << endl;
    printf("done\n");
    getchar();
  }
  exit(0);
  
  int max_steps = 32;
  
  srand(time(0));
  
  int steps = max_steps;

  Field f;
  while (1) {
    int choice = rand() % 8;
    if (choice == 0) {
      turn(f, 'B');
    } else {
      turn(f, '0' + choice);
    }
    cerr << steps << " " << f.score() << endl;
    steps--;
    if (steps == 0) {
      f.add_B_row();
      steps = --max_steps;
    }
  }

}
