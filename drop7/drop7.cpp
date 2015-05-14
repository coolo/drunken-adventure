#include "opencv2/opencv.hpp"
#include "Field.h"
#include <iostream>
#include <limits>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

using namespace cv;
using namespace std;

void turn(Field &f, char t)
{
  double min = std::numeric_limits<double>::max();
  int col = -1;
  for (int x = 0; x < 7; x++) {
    Field droped = f.drop(t, x + 1);
    //cerr << droped.to_string();
    //cerr << endl << endl;

    double r = droped.recursive_rating(3);
    //cerr << x << " " << r << endl;
    if (min > r) {
      col = x + 1;
      min = r;
    }
  }
  f = f.drop(t, col);
  f.finalize_random();
  while (f.blink()) {
    f.finalize_random();
  }

  cerr << f.to_string();
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
	  // reduce the colours to 16 before checking the diff
	  if (abs(I1_data[i] - I2_data[i]) < 16)
	    continue; // += 0
	  double t1 = round(I1_data[i] / 16.);
	  double t2 = round(I2_data[i] / 16.);
	  double diff = (t1 - t2) * 16;
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

int main()
{
  buttons.push_back(Button('1', "1.png"));
  buttons.push_back(Button('1', "1-2.png"));
  buttons.push_back(Button('1', "1-3.png"));
  buttons.push_back(Button('1', "1-4.png"));
  buttons.push_back(Button('1', "1-5.png"));
  buttons.push_back(Button('2', "2.png"));
  buttons.push_back(Button('3', "3.png"));
  buttons.push_back(Button('3', "3-2.png"));
  buttons.push_back(Button('4', "4.png"));
  buttons.push_back(Button('4', "4-2.png"));
  buttons.push_back(Button('5', "5.png"));
  buttons.push_back(Button('6', "6.png"));
  buttons.push_back(Button('6', "6-2.png"));
  buttons.push_back(Button('6', "6-3.png"));
  buttons.push_back(Button('7', "7.png"));
  buttons.push_back(Button('7', "7-2.png"));
  buttons.push_back(Button('A', "A.png"));
  buttons.push_back(Button('A', "A-2.png"));
  buttons.push_back(Button('A', "A-3.png"));
  buttons.push_back(Button('B', "B.png"));
  buttons.push_back(Button('B', "B-2.png"));

  Field f;
  
  Mat frame = resized_screencap();

  int min_x = 13;
  int min_y = 79;
  int part_size = 25;
  
  for (int y = 0; y < 7; y++) {
    for (int x = 0; x < 7; x++) {
      std::vector<Button>::const_iterator it = buttons.begin();
      Mat part(frame, Rect(part_size * x + min_x + 5, (part_size + 2) * y + min_y + 5, part_size - 10, part_size - 10 ));
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
      if (maxmse < 30)
	f.set(y, x, maxletter);
      else {
	cerr << y << " " << x << " " << maxletter << " " << maxmse << endl;
	char buffer[300];
	sprintf(buffer, "part-%dx%d-%lf-%c.png", y, x, maxmse, maxletter);
	imwrite(buffer, part);
      }
    }
  }

  cerr << f.to_string();
  
  exit(0);
  
  int max_steps = 32;
  
  srand(time(0));
  
  int steps = max_steps;
  
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
