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

using namespace cv;
using namespace std;

double dX = .749;

int humming_distance(unsigned long hash, unsigned long ref)
{
  int weight = 0;
  unsigned long word = hash ^ ref;
  
  while (word) {
    if (word & 1) weight++; 
    word >>= 1;
  }
  return weight; 
}

int main(int argc, char **argv)
{
  Mat m, hsv, chan[3];

  Mat img = imread(argv[1]);

  cvtColor(img, hsv, CV_BGR2HSV);
  split(img, chan);
  cvtColor(img, m, CV_BGR2GRAY);
  m.convertTo(m, CV_8UC1);

  Mat cool(8 * 44, 9 * 44, CV_8UC3);
  
  double delta = 19.34;
  
  for (int py = 0; py < 44; py++) {
    for (int px = 0; px < 44; px++) {

      // see https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
      double a = -dX;
      double c = 564 * dX + py * delta;
      double b = dX;
      double d = (44 - px) * delta - 799 * dX;

      uint64_t hash = 0;
      for (int y = 0; y < 8; y++) {
	int prev = -1;
	for (int x = 0; x < 9; x++) {
	  double ix2 = (d - delta * x / 9. - c) / (a - b);
	  double iy2 = a * ix2 + c;
	  if (iy2 < 0 || iy2 > 630) // the last lines are troops
	    continue;
	  if (ix2 < 0 || ix2 > img.cols)
	    continue;
	  // possible improvement: interpolation
	  uchar pixel = img.at<Vec3b>(iy2, ix2)[1];
	  cool.at<Vec3b>(py * 8 + y, px * 9 + x) = Vec3b(pixel, pixel, pixel);
	  if (prev != -1) {
	    bool bit = pixel < prev;
	    hash += bit;
	    hash <<= 1;
	  }
	  prev = pixel;
	}
	c += delta / 8;
      }
      //printf("%d %016lx %d\n", py, hash, humming_distance(hash, 0xe4c5f1e1c5c0e4e0));
      int dist = humming_distance(hash, 0xe4c5f1e1c5c0e4e0);
      for (int y = 0; y < 8; y++)
	for (int x = 0; x < 9; x++) {
	  uchar pixel = cool.at<Vec3b>(py * 8 + y, px * 9 + x)[1];
	  cool.at<Vec3b>(py * 8 + y, px * 9 + x) = Vec3b(pixel, pixel, 255 - dist * 4);
	}
    }
  }
  imwrite("hsv.png", cool);
}
