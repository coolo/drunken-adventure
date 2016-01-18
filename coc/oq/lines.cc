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

const int palette_size = 250;
Vec3f palette[palette_size+1];

void initPalette() {

  palette[0] = Vec3f(23, 142, 113);
  palette[1] = Vec3f(27, 150, 103);
  palette[2] = Vec3f(35, 161, 93);
  palette[3] = Vec3f(48, 153, 104);
  palette[4] = Vec3f(43, 144, 113);
  palette[5] = Vec3f(38, 135, 122);
  palette[6] = Vec3f(19, 135, 125);
  palette[7] = Vec3f(18, 130, 130);
  palette[8] = Vec3f(31, 130, 139);
  palette[9] = Vec3f(30, 121, 144);
  palette[10] = Vec3f(36, 131, 129);
  palette[11] = Vec3f(45, 128, 136);
  palette[12] = Vec3f(54, 128, 134);
  palette[13] = Vec3f(55, 131, 126);
  palette[14] = Vec3f(50, 136, 121);
  palette[15] = Vec3f(59, 136, 119);
  palette[16] = Vec3f(64, 129, 130);
  palette[17] = Vec3f(71, 134, 121);
  palette[18] = Vec3f(74, 129, 131);
  palette[19] = Vec3f(80, 129, 130);
  palette[20] = Vec3f(88, 130, 127);
  palette[21] = Vec3f(86, 135, 117);
  palette[22] = Vec3f(81, 148, 116);
  palette[23] = Vec3f(64, 148, 114);
  palette[24] = Vec3f(65, 154, 102);
  palette[25] = Vec3f(57, 166, 94);
  palette[26] = Vec3f(50, 177, 66);
  palette[27] = Vec3f(65, 179, 68);
  palette[28] = Vec3f(91, 189, 68);
  palette[29] = Vec3f(90, 165, 92);
  palette[30] = Vec3f(78, 169, 100);
  palette[31] = Vec3f(82, 153, 103);
  palette[32] = Vec3f(97, 154, 100);
  palette[33] = Vec3f(97, 150, 113);
  palette[34] = Vec3f(102, 137, 114);
  palette[35] = Vec3f(98, 128, 125);
  palette[36] = Vec3f(89, 129, 134);
  palette[37] = Vec3f(65, 127, 139);
  palette[38] = Vec3f(62, 125, 144);
  palette[39] = Vec3f(47, 126, 147);
  palette[40] = Vec3f(44, 120, 148);
  palette[41] = Vec3f(61, 121, 149);
  palette[42] = Vec3f(62, 116, 155);
  palette[43] = Vec3f(74, 118, 157);
  palette[44] = Vec3f(235, 113, 176);  
  palette[45] = Vec3f(75, 124, 149);
  palette[46] = Vec3f(75, 126, 142);
  palette[47] = Vec3f(86, 127, 140);
  palette[48] = Vec3f(86, 124, 146);
  palette[49] = Vec3f(89, 119, 156);
  palette[50] = Vec3f(85, 112, 161);
  palette[51] = Vec3f(212, 120, 174);
  palette[52] = Vec3f(99, 110, 166);
  palette[53] = Vec3f(102, 117, 163);
  palette[54] = Vec3f(98, 122, 140);
  palette[55] = Vec3f(106, 132, 141);
  palette[56] = Vec3f(112, 134, 128);
  palette[57] = Vec3f(117, 129, 118);
  palette[58] = Vec3f(114, 141, 113);
  palette[59] = Vec3f(111, 151, 102);
  palette[60] = Vec3f(123, 154, 105);
  palette[61] = Vec3f(126, 143, 108);
  palette[62] = Vec3f(142, 139, 114);
  palette[63] = Vec3f(142, 132, 124);
  palette[64] = Vec3f(134, 131, 127);
  palette[65] = Vec3f(116, 122, 133);
  palette[66] = Vec3f(116, 118, 145);
  palette[67] = Vec3f(103, 122, 157);
  palette[68] = Vec3f(108, 112, 166);
  palette[69] = Vec3f(115, 113, 167);
  palette[70] = Vec3f(118, 105, 172);
  palette[71] = Vec3f(116, 95, 174);
  palette[72] = Vec3f(128, 99, 177);
  palette[73] = Vec3f(131, 109, 171);
  palette[74] = Vec3f(124, 114, 167);
  palette[75] = Vec3f(112, 119, 164);
  palette[76] = Vec3f(113, 125, 156);
  palette[77] = Vec3f(114, 131, 146);
  palette[78] = Vec3f(114, 133, 138);
  palette[79] = Vec3f(110, 141, 137);
  palette[80] = Vec3f(35, 147, 139);
  palette[81] = Vec3f(56, 150, 148);
  palette[82] = Vec3f(57, 154, 141);
  palette[83] = Vec3f(59, 142, 151);
  palette[84] = Vec3f(68, 142, 150);
  palette[85] = Vec3f(78, 142, 149);
  palette[86] = Vec3f(74, 151, 147);
  palette[87] = Vec3f(73, 154, 139);
  palette[88] = Vec3f(50, 160, 152);
  palette[89] = Vec3f(73, 169, 158);
  palette[90] = Vec3f(75, 139, 158);
  palette[91] = Vec3f(218, 113, 185);
  palette[92] = Vec3f(88, 138, 164);
  palette[93] = Vec3f(90, 145, 152);
  palette[94] = Vec3f(96, 146, 149);
  palette[95] = Vec3f(89, 153, 139);
  palette[96] = Vec3f(102, 152, 134);
  palette[97] = Vec3f(105, 146, 147);
  palette[98] = Vec3f(102, 143, 156);
  palette[99] = Vec3f(102, 140, 163);
  palette[100] = Vec3f(102, 135, 171);
  palette[101] = Vec3f(111, 136, 155);
  palette[102] = Vec3f(124, 132, 133);
  palette[103] = Vec3f(128, 140, 129);
  palette[104] = Vec3f(140, 129, 135);
  palette[105] = Vec3f(142, 128, 145);
  palette[106] = Vec3f(132, 135, 144);
  palette[107] = Vec3f(124, 135, 148);
  palette[108] = Vec3f(124, 130, 160);
  palette[109] = Vec3f(125, 122, 162);
  palette[110] = Vec3f(134, 121, 157);
  palette[111] = Vec3f(132, 130, 157);
  palette[112] = Vec3f(143, 122, 154);
  palette[113] = Vec3f(143, 92, 162);
  palette[114] = Vec3f(158, 88, 165);
  palette[115] = Vec3f(159, 101, 164);
  palette[116] = Vec3f(138, 111, 171);
  palette[117] = Vec3f(140, 118, 165);
  palette[118] = Vec3f(148, 112, 171);
  palette[119] = Vec3f(144, 103, 177);
  palette[120] = Vec3f(144, 97, 183);
  palette[121] = Vec3f(158, 100, 182);
  palette[122] = Vec3f(166, 100, 182);
  palette[123] = Vec3f(160, 108, 176);
  palette[124] = Vec3f(160, 114, 168);
  palette[125] = Vec3f(154, 118, 159);
  palette[126] = Vec3f(154, 123, 150);
  palette[127] = Vec3f(151, 124, 133);
  palette[128] = Vec3f(153, 129, 128);
  palette[129] = Vec3f(153, 132, 117);
  palette[130] = Vec3f(158, 139, 100);
  palette[131] = Vec3f(151, 151, 104);
  palette[132] = Vec3f(138, 158, 103);
  palette[133] = Vec3f(127, 169, 111);
  palette[134] = Vec3f(80, 168, 113);
  palette[135] = Vec3f(83, 177, 93);
  palette[136] = Vec3f(96, 169, 115);
  palette[137] = Vec3f(106, 184, 102);
  palette[138] = Vec3f(118, 182, 93);
  palette[139] = Vec3f(114, 197, 77);
  palette[140] = Vec3f(128, 198, 73);
  palette[141] = Vec3f(141, 190, 80);
  palette[142] = Vec3f(146, 172, 95);
  palette[143] = Vec3f(114, 172, 113);
  palette[144] = Vec3f(119, 154, 130);
  palette[145] = Vec3f(141, 161, 127);
  palette[146] = Vec3f(147, 159, 151);
  palette[147] = Vec3f(137, 159, 156);
  palette[148] = Vec3f(129, 160, 156);
  palette[149] = Vec3f(74, 158, 164);
  palette[150] = Vec3f(94, 161, 167);
  palette[151] = Vec3f(93, 170, 166);
  palette[152] = Vec3f(91, 172, 157);
  palette[153] = Vec3f(107, 173, 159);
  palette[154] = Vec3f(109, 166, 170);
  palette[155] = Vec3f(121, 167, 167);
  palette[156] = Vec3f(121, 172, 155);
  palette[157] = Vec3f(128, 156, 165);
  palette[158] = Vec3f(128, 153, 172);
  palette[159] = Vec3f(121, 141, 175);
  palette[160] = Vec3f(135, 147, 180);
  palette[161] = Vec3f(139, 132, 184);
  palette[162] = Vec3f(148, 127, 187);
  palette[163] = Vec3f(157, 90, 191);
  palette[164] = Vec3f(173, 98, 186);
  palette[165] = Vec3f(175, 106, 180);
  palette[166] = Vec3f(170, 109, 177);
  palette[167] = Vec3f(165, 112, 159);
  palette[168] = Vec3f(166, 118, 152);
  palette[169] = Vec3f(165, 121, 141);
  palette[170] = Vec3f(168, 128, 128);
  palette[171] = Vec3f(170, 134, 123);
  palette[172] = Vec3f(169, 136, 111);
  palette[173] = Vec3f(188, 134, 111);
  palette[174] = Vec3f(193, 147, 102);
  palette[175] = Vec3f(178, 159, 106);
  palette[176] = Vec3f(165, 172, 92);
  palette[177] = Vec3f(157, 191, 79);
  palette[178] = Vec3f(209, 134, 110);
  palette[179] = Vec3f(197, 131, 125);
  palette[180] = Vec3f(191, 132, 132);
  palette[181] = Vec3f(186, 132, 136);
  palette[182] = Vec3f(181, 141, 142);
  palette[183] = Vec3f(165, 138, 148);
  palette[184] = Vec3f(171, 148, 141);
  palette[185] = Vec3f(160, 153, 147);
  palette[186] = Vec3f(158, 148, 155);
  palette[187] = Vec3f(161, 147, 164);
  palette[188] = Vec3f(160, 139, 172);
  palette[189] = Vec3f(152, 139, 176);
  palette[190] = Vec3f(144, 145, 174);
  palette[191] = Vec3f(145, 141, 182);
  palette[192] = Vec3f(149, 155, 167);
  palette[193] = Vec3f(141, 158, 174);
  palette[194] = Vec3f(122, 162, 176);
  palette[195] = Vec3f(209, 116, 195);
  palette[196] = Vec3f(140, 176, 154);
  palette[197] = Vec3f(146, 177, 175);
  palette[198] = Vec3f(135, 188, 173);
  palette[199] = Vec3f(110, 189, 177);
  palette[200] = Vec3f(124, 190, 178);
  palette[201] = Vec3f(136, 184, 184);
  palette[202] = Vec3f(146, 171, 190);
  palette[203] = Vec3f(159, 176, 170);
  palette[204] = Vec3f(172, 139, 168);
  palette[205] = Vec3f(175, 135, 158);
  palette[206] = Vec3f(176, 113, 155);
  palette[207] = Vec3f(186, 127, 150);
  palette[208] = Vec3f(182, 134, 150);
  palette[209] = Vec3f(197, 128, 142);
  palette[210] = Vec3f(207, 123, 147);
  palette[211] = Vec3f(210, 129, 128);
  palette[212] = Vec3f(221, 129, 119);
  palette[213] = Vec3f(222, 124, 138);
  palette[214] = Vec3f(238, 126, 128);
  palette[215] = Vec3f(237, 119, 149);
  palette[216] = Vec3f(223, 117, 159);
  palette[217] = Vec3f(208, 120, 162);
  palette[218] = Vec3f(199, 123, 162);
  palette[219] = Vec3f(195, 128, 170);
  palette[220] = Vec3f(188, 129, 174);
  palette[221] = Vec3f(178, 111, 175);
  palette[222] = Vec3f(183, 109, 179);
  palette[223] = Vec3f(207, 127, 204);
  palette[224] = Vec3f(184, 102, 185);
  palette[225] = Vec3f(180, 95, 191);
  palette[226] = Vec3f(194, 101, 184);
  palette[227] = Vec3f(195, 91, 195);
  palette[228] = Vec3f(206, 59, 204);
  palette[229] = Vec3f(213, 84, 202);
  palette[230] = Vec3f(225, 78, 205);
  palette[231] = Vec3f(215, 97, 199);
  palette[232] = Vec3f(218, 107, 193);
  palette[233] = Vec3f(192, 113, 188);
  palette[234] = Vec3f(186, 116, 192);
  palette[235] = Vec3f(181, 116, 196);
  palette[236] = Vec3f(163, 120, 193);
  palette[237] = Vec3f(160, 127, 184);
  palette[238] = Vec3f(161, 137, 186);
  palette[239] = Vec3f(174, 137, 179);
  palette[240] = Vec3f(177, 131, 184);
  palette[241] = Vec3f(231, 97, 200);
  palette[242] = Vec3f(183, 135, 169);
  palette[243] = Vec3f(195, 125, 186);
  palette[244] = Vec3f(185, 125, 191);
  palette[245] = Vec3f(176, 123, 193);
  palette[246] = Vec3f(176, 142, 199);
  palette[247] = Vec3f(172, 154, 191);
  palette[248] = Vec3f(161, 162, 192);
  palette[249] = Vec3f(202, 119, 179);

}

inline int humming_distance(unsigned long hash, unsigned long ref)
{
  return __builtin_popcountll(hash ^ ref);
}

long hashes[44][44];
Mat matrix;

inline cv::Vec3f getColorSubpix(const cv::Mat& img, double dy, double dx)
{
    assert(!img.empty());
    assert(img.channels() == 3);

    int x = (int)dx;
    int y = (int)dy;

    int x0 = cv::borderInterpolate(x,   img.cols, cv::BORDER_REFLECT_101);
    int x1 = cv::borderInterpolate(x+1, img.cols, cv::BORDER_REFLECT_101);
    int y0 = cv::borderInterpolate(y,   img.rows, cv::BORDER_REFLECT_101);
    int y1 = cv::borderInterpolate(y+1, img.rows, cv::BORDER_REFLECT_101);

    float a = dx - (float)x;
    float c = dy - (float)y;

    float b = cvRound((img.at<cv::Vec3f>(y0, x0)[0] * (1.f - a) + img.at<cv::Vec3f>(y0, x1)[0] * a) * (1.f - c)
		      + (img.at<cv::Vec3f>(y1, x0)[0] * (1.f - a) + img.at<cv::Vec3f>(y1, x1)[0] * a) * c);
    float g = cvRound((img.at<cv::Vec3f>(y0, x0)[1] * (1.f - a) + img.at<cv::Vec3f>(y0, x1)[1] * a) * (1.f - c)
		      + (img.at<cv::Vec3f>(y1, x0)[1] * (1.f - a) + img.at<cv::Vec3f>(y1, x1)[1] * a) * c);
    float r = cvRound((img.at<cv::Vec3f>(y0, x0)[2] * (1.f - a) + img.at<cv::Vec3f>(y0, x1)[2] * a) * (1.f - c)
		      + (img.at<cv::Vec3f>(y1, x0)[2] * (1.f - a) + img.at<cv::Vec3f>(y1, x1)[2] * a) * c);

    return cv::Vec3f(b, g, r);
}

int find_best_match(const Vec3f &f)
{
  double min_distance = INFINITY;
  int best_match = 0;
  for (int p = 0; p < palette_size; ++p) {
    double distance = norm(f - palette[p]);
    if (distance < min_distance) {
      min_distance = distance;
      best_match = p;
    }
  }
  return best_match;
}

const int x_parts = 8;
const int y_parts = 8;

Mat split_img;

Mat translate_to_hashes(const Mat &img)
{
  Mat lab;
  cvtColor(img, lab, CV_BGR2Lab);
  lab.convertTo(lab, CV_32F);
  
  split_img = Mat((y_parts + 2) * 44, (x_parts + 2) * 44, CV_32FC3);
  Mat matrix(y_parts * 44, x_parts * 44, CV_8UC1);
  split_img = Scalar::all(0);
  
  for (int py = 0; py < 44; py++) {
    for (int px = 0; px < 44; px++) {

      // see https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
      double a = -dX;
      double c = 564 * dX + py * delta;
      double b = dX;
      double d = (44 - px) * delta - 799 * dX;

      for (int y = 0; y < y_parts; y++) {
	for (int x = 0; x < x_parts; x++) {
	  double ix2 = (d - delta * x / double(x_parts) - c) / (a - b);
	  double iy2 = a * ix2 + c;
	  if (iy2 < 0 || iy2 > 640) // the last lines are troops
	    continue;
	  if (ix2 < 0 || ix2 > img.cols)
	    continue;
	  int pixel = find_best_match(getColorSubpix(lab, iy2, ix2));
	  //cout << pixel[0] << " " << pixel[1] << " " << pixel[2] << endl;
	  split_img.at<Vec3f>(py * (y_parts + 2) + y, px * (x_parts + 2) + x) = palette[pixel];
	  matrix.at<uchar>(py * y_parts + y, px * x_parts + x) = pixel;
	}
	c += delta / y_parts;
      }
    }
  }
  
  split_img.convertTo(split_img, CV_8UC3);
  cvtColor(split_img, split_img, CV_Lab2BGR);
  imwrite("hsv.png", split_img);
  imwrite("matrix.png", matrix);
  //imshow("image", cool);

  return matrix;
}

Mat mark_object(const Mat &_img, int ty, int tx, int size)
{
  cout << "MARK " << ty << " " << tx << endl;
  Mat cool = _img.clone();
  for (int py = ty; py < ty + size; py++) {
    for (int px = tx; px < tx + size; px++) {
      for (int y = 0; y < cool.rows; y++)
	for (int x = 0; x < cool.cols; x++) {
	  double a = -dX;
	  double c = 564 * dX + py * delta;
	  double b = dX;
	  double d = (44 - px) * delta - 799 * dX;
	  
	  if (y < a * x + c || y > a * x + c + delta)
	    continue;

	  if (y < b * x + d - delta || y > b * x + d)
	    continue;

	  Vec3b pixel = cool.at<Vec3b>(y, x);
	  pixel[0] = 256 - pixel[0];
	  pixel[1] = 256 - pixel[1];
	  pixel[2] = 256 - pixel[2];
	  cool.at<Vec3b>(y, x) = pixel;
	}
    }
  }

  for (int py = ty; py < ty + size; py++) {
    for (int px = tx; px < tx + size; px++) {
      for (int y = 0; y < y_parts; y++) {
	for (int x = 0; x < x_parts; x++) {
	  Vec3b pixel = split_img.at<Vec3b>(py * (y_parts + 2) + y, px * (x_parts + 2) + x);
	  pixel[0] = 255;
	  pixel[1] = 255;
	  pixel[2] = 255;
	  split_img.at<Vec3b>(py * (y_parts + 2) + y, px * (x_parts + 2) + x) = pixel;
	}
      }
    }
  }
  imwrite("marked.png", split_img);
  return cool;
}

struct COCObject {
  string name;
  int size;
  uchar hashes[5*5*y_parts*x_parts];
};

// 1 for right top corner, < 0.3 for the farest away
double distance_factor(int y, int x, int size)
{
  x = size - 1 - x;
  int dy = y - size;
  int dx = x - size;
  double dist = sqrt(double(dy * dy + dx * dx) / (size * size * 2));
  return dist;
}

int verbose = 0;

double object_distance(const COCObject &o, int y, int x)
{
  //  cout << "object_distance " << y <<  " " << x << endl;
  double distance = 0;
  int pixel_index = 0;
  for (int oy = 0; oy < o.size; oy++) {
    for (int ox = 0; ox < o.size; ox++) {
      int xi = x * x_parts;
      double pdistance = 0;
      double f = distance_factor(oy, ox, o.size);
      for (int ty = 0; ty < y_parts; ty++) {
	for (int tx = 0; tx < x_parts; tx++) {
	  
	  uchar pixel = matrix.at<uchar>((oy + y) * y_parts + ty, xi++);
	  uchar ref = o.hashes[pixel_index++];
	  if (verbose)
	    printf("%d(%d) %d(%d) %d-%d\n", oy, ty, ox, tx, int(pixel), int(ref));
	  //cout << oy << "(" << << " " << ox << " " << f << " " << int(pixel) << " " << int(ref) << endl;
	  if (pixel >= palette_size) {
	    distance += INT_MAX;
	  } else {
	    double n = norm(palette[pixel] - palette[ref]);
	    //cout << "NORM " << n << endl;
	    distance += n * n;
	    pdistance += n * n;
	    //distance +=  * f;
	  }
	}
      }
      cout << "PART " << oy << " " << ox << " " << long(pdistance) << " " << f << endl;
    }
  }
  //cout << distance << "\n\n";
  return sqrt(distance / x_parts / y_parts / o.size / o.size);
}

vector<COCObject> objects;

void print_object(const COCObject &to, int ty, int tx)
{
  printf("\nObject %s\n", to.name.c_str());
  double distance = 0;
  for (int oy = 0; oy < to.size; oy++) {
    for (int ox = 0; ox < to.size; ox++) {
#if 0
      int delta = humming_distance(to.hashes[oy * to.size + ox], hashes[ty + oy][tx + ox]);
      double f = distance_factor(oy, ox, to.size);
      printf("%016lx-%016lx(%d*%f) ", hashes[oy + ty][ox + tx], to.hashes[oy * to.size + ox], delta, f);
      distance += delta * f;
#endif
    }
    printf("Distance: %f\n", distance);
  }
}

double find_object(int &ty, int &tx, COCObject &to)
{
  double best_distance = INT_MAX;
  vector<COCObject>::const_iterator it = objects.begin();
  for (; it != objects.end(); ++it) {
    double best_obj_dist = INT_MAX;
    int box = 0;
    int boy = 0;
    for (int y = 0; y < 45 - it->size; y++)
      for (int x = 0; x < 45 - it->size; x++) {
	double distance = object_distance(*it, y, x);
	if (distance < best_distance) {
	  best_distance = distance;
	  ty = y;
	  tx = x;
	  to = *it;
	}
	if (distance < best_obj_dist) {
	  best_obj_dist = distance;
	  box = x;
	  boy = y;
	}
    }
    cout << "best obj " << it->name << " " << long(best_obj_dist) << " " << boy << " " << box << endl;
   
  }
  verbose = 1;
  cout << "best " << to.name << " " << long(best_distance) << endl;
  object_distance(to, ty, tx);
  verbose = 0;
  Mat s(to.size * y_parts, to.size * x_parts, CV_32FC3);
  for (int py = ty; py < ty + to.size; py++)
    for (int y1 = 0; y1 < y_parts; y1++)
      for (int px = tx; px < tx + to.size; px++)
	for (int x1 = 0; x1 < x_parts; x1++) {
	  uchar pixel = matrix.at<uchar>(py * y_parts + y1, px * x_parts + x1);
	  s.at<Vec3f>((py-ty) * y_parts + y1, (px-tx) * x_parts + x1) = palette[pixel];
	}
  s.convertTo(s, CV_8UC3);
  cvtColor(s, s, CV_Lab2BGR);
  imwrite("best.png", s);
  
  //print_object(to, ty, tx);
  return best_distance;
}

int hex2int(char c) {
  if (c >= 'a')
    return c - 'a' + 10;
  return c - '0';
}

void read_objects(const char *filename)
{
  FILE *f = fopen(filename, "r");
  if (!f)
    return;
  
  char buffer[10000];
  while (fgets(buffer, sizeof(buffer) - 1, f)) {
    COCObject o;
    int count = 0;
    const char *c = buffer;
    while (*c && *c != '\n') {
      o.hashes[count++] = (hex2int(c[0]) << 4) + hex2int(c[1]);
      c += 2;
    }
    o.size = sqrt(count / x_parts / y_parts);
    if (!o.size)
      continue;
    o.name = filename;
    objects.push_back(o);
  }
  fclose(f);
}

int mark_all_objects(Mat &img, int &y, int &x, COCObject &o)
{
  double distance;
  while (1) {
    distance = find_object(y, x, o);
    if ( distance >= 25 )
      return distance;
    img = mark_object(img, y, x, o.size);
    cout << "MARK " << o.name << " " << y << " " << x << " "  << distance << endl;
    //imshow("image", img);
    //waitKey(0);
    for (int py = 0; py < o.size; py++)
      for (int px = 0; px < o.size; px++)
	matrix.at<uchar>((y+py) * y_parts, (x+px) * x_parts) = palette_size;
  }
}

int main(int argc, char **argv)
{
  initPalette();
  int argv_index = 2;
  Mat img = imread(argv[argv_index]);
  matrix = translate_to_hashes(img);
  
  namedWindow("image");
 
  DIR *dir = opendir("hashes");
  struct dirent *entry;
  while ((entry = readdir(dir))) {
    char buffer[PATH_MAX];
    sprintf(buffer, "hashes/%s", entry->d_name);
    read_objects(buffer);
  }
  closedir(dir);
  
  int y = 22;
  int x = 22;
  COCObject o;
  o.size = 2;
  o.name = argv[1];
  objects.push_back(o);
  
  int distance = mark_all_objects(img, y, x, o);
  //print_object(o, y, x);
  
  while (1) {
    //long argv_hash = hashes[y][x];
    cout << "Y " << y << " X " << x << endl;
    verbose = 1;
    object_distance(o, y, x);
    verbose = 0;
    Mat cool = mark_object(img, y, x, o.size);
    vector<COCObject>::const_iterator it = objects.begin();
    for (; it != objects.end(); ++it) {
      double distance = object_distance(*it, y, x);
      cout << it->name << " distance " << long(distance) << endl;
    }
      
    int key = 0;
    while (key <= 0) {
      imshow("image",cool);
      key = waitKey(400);
      if (key > 0)
	break;
      imshow("image", img);
      key = waitKey(400);
    }
    //printf("Key %d\n", key);
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
    if (key == 'n') {
      img = imread(argv[++argv_index]);
      matrix = translate_to_hashes(img);
      distance = mark_all_objects(img, y, x, o);
    }
    if (key == ' ') {
      printf("%d: \n", distance);
      Mat s(o.size * y_parts, o.size * x_parts, CV_32FC3);
      FILE *f = fopen(argv[1], "a");
      int pixel_index = 0;
      for (int py = y; py < y + o.size; py++)
	for (int ty = 0; ty < y_parts; ty++)
	  for (int px = x; px < x + o.size; px++)
	    for (int tx = 0; tx < x_parts; tx++) {
	      uchar pixel = matrix.at<uchar>(py * y_parts + ty, px * x_parts + tx);
	      s.at<Vec3f>((py-y) * y_parts + ty, (px-x) * x_parts + tx) = palette[pixel];
	      o.hashes[pixel_index++] = pixel;
	      fprintf(f, "%02x", pixel);
	    }
      s.convertTo(s, CV_8UC3);
      cvtColor(s, s, CV_Lab2BGR);
      imwrite("s.png", s);
      fprintf(f,"\n");
      fclose(f);
      objects.push_back(o);
      img = imread(argv[++argv_index]);
      matrix = translate_to_hashes(img);
      distance = mark_all_objects(img, y, x, o);
    }
  }
}
