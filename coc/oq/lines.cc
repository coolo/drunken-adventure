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
double delta = 19.34;

int dist_limit = 14;

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

long hashes[44][44];

Mat translate_to_hashes(const Mat &img)
{
  Mat cool(8 * 44, 9 * 44, CV_8UC3);
  
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
	  double dx = ix2 - floor(ix2);
	  double Q0 = (1 - dx) * img.at<Vec3b>(iy2, ix2)[1];
	  Q0 += dx * img.at<Vec3b>(iy2, ix2 + 1)[1];
	  double Q1 = (1 - dx) * img.at<Vec3b>(iy2 + 1, ix2)[1];
	  Q1 += dx * img.at<Vec3b>(iy2 + 1, ix2 + 1)[1];
	  // possible improvement: interpolation
	  double dy = iy2 - floor(iy2);
	  uchar pixel = (1 - dy) * Q0 + dy * Q1 + 0.5;
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
      hashes[py][px] = hash;
    }
  }
  return cool;
}

int mark_hash(const Mat &_cool, long argv_hash)
{
  Mat cool = _cool.clone();
  for (int py = 0; py < 44; py++) {
    for (int px = 0; px < 44; px++) {
      long hash = hashes[py][px];
      //printf("%016lx\n", argv_hash);
      int dist = humming_distance(hash, argv_hash);
      for (int y = 0; y < 8; y++)
	for (int x = 0; x < 9; x++) {
	  uchar pixel = cool.at<Vec3b>(py * 8 + y, px * 9 + x)[1];
          if (dist < dist_limit) {
            cool.at<Vec3b>(py * 8 + y, px * 9 + x) = Vec3b(pixel, 255 - dist * 4, pixel);
          }
	  
	}
    }
  }
  imwrite("hsv.png", cool);
  namedWindow("image");
  imshow("image",cool);
  return 0;
}

int mark_object(const Mat &_img, long argv_hash)
{
  Mat cool = _img.clone();
  for (int py = 0; py < 44; py++) {
    for (int px = 0; px < 44; px++) {
      long hash = hashes[py][px];
      //printf("%016lx\n", argv_hash);
      int dist = humming_distance(hash, argv_hash);
      if (dist > dist_limit) {
	continue;
      }
      for (int y = 0; y < cool.rows; y++)
	for (int x = 0; x < cool.cols; x++) {
	  double a = -dX;
	  double c = 564 * dX + py * delta;
	  double b = dX;
	  double d = (44 - px) * delta - 799 * dX;
	  
	  if (y < a * x + c || y > a * x + c + delta)
	    continue;

	  if (y < b * x + d || y > b * x + d + delta)
	    continue;

	  Vec3b pixel = cool.at<Vec3b>(y, x);
	  pixel[0] = 256 - pixel[0];
	  pixel[1] = 256 - pixel[1];
	  pixel[2] = 256 - pixel[2];
	  cool.at<Vec3b>(y, x) = pixel;
	}
    }
  }
  imwrite("hsv.png", cool);
  namedWindow("image");
  imshow("image",cool);
  return 0;
}

int main(int argc, char **argv)
{
  Mat img = imread(argv[1]);

  Mat cool = translate_to_hashes(img);

  int y = 22;
  int x = 22;

  while (1) {
    long argv_hash = hashes[y][x];
    printf("HASH %lx %d\n", argv_hash, dist_limit);
    mark_object(img, argv_hash);
    
    int key = waitKey(0);
    printf("Key %d\n", key);
    if (key == 65361) {
      x--;
    }
    if (key == 65362) {
      y--;
    }
    if (key == 65364) {
      y++;
    }
    if (key == 65363) {
      x++;
    }
    if (key == 'a') {
      dist_limit++;
    }
    if (key == 's') {
      dist_limit--;
    }
  }
}
