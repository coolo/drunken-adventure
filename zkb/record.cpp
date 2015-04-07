#include "opencv2/opencv.hpp"
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr

#include <iostream>

using namespace cv;
using namespace std;

const int HUMAN_LOOK_ALIKE = 770;

int do_loop = 1;
int do_adb = 1;

int monkey_socket = -1;
bool screenrecord = true;

Mat frame;

// 192 / cols
double scale = 1;

int min_y = 77;
int cut_y = 0;


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
    cols = object.cols;
    rows = object.rows;
    object.convertTo(eightbit, CV_8UC1);
    letter = _letter;
  }
};

void sig_handler(int signum)
{
  printf("Received signal %d\n", signum);
  do_loop = 0;
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

/* we find the object in the scene and return the x,y and the error of the match */
Point2i image_search(const Mat &scene, const Button &button) {

  const Mat object = button.eightbit;
  
  int result_width  = scene.cols - object.cols + 1;
  int result_height = scene.rows - object.rows + 1;
  Mat result = Mat::zeros(result_height, result_width, CV_32FC1);

  // Perform the matching. Info about algorithm:
  // http://docs.opencv.org/trunk/doc/tutorials/imgproc/histograms/template_matching/template_matching.html
  // http://docs.opencv.org/modules/imgproc/doc/object_detection.html
  matchTemplate(scene, object, result, CV_TM_SQDIFF_NORMED);

  // Localizing the best match with minMaxLoc
  double minval, maxval;
  Point  minloc, maxloc;
  minMaxLoc(result, &minval, &maxval, &minloc, &maxloc, Mat());

  Mat test(scene, Rect(minloc.x, minloc.y, object.cols, object.rows));
  double mse = enhancedMSE(object, Mat(scene, Rect(minloc.x, minloc.y, object.cols, object.rows)));
  char fname[300];
  sprintf(fname, "%c-result.png", button.letter);
  imwrite(fname, test);
  if (mse < 1000)
    return minloc;
  return Point(0,0);
}

vector<Button> buttons;
typedef Matx<char, 8, 8> Zookeeper;

vector<Button> extras;

void init_buttons() {
  buttons.push_back(Button('E', "data/E.png"));
  buttons.push_back(Button('L', "data/L.png"));
  buttons.push_back(Button('A', "data/A.png"));
  buttons.push_back(Button('C', "data/C.png"));
  buttons.push_back(Button('F', "data/F.png"));
  buttons.push_back(Button('G', "data/G.png"));
  buttons.push_back(Button('H', "data/H.png"));
  buttons.push_back(Button('P', "data/P.png"));
  buttons.push_back(Button('+', "data/AllP.png"));
  buttons.push_back(Button('+', "data/AllA.png"));
  buttons.push_back(Button('+', "data/All4.png"));
  buttons.push_back(Button('+', "data/AllL.png"));
  buttons.push_back(Button('+', "data/AllG.png"));
  buttons.push_back(Button('+', "data/AllH.png"));
  buttons.push_back(Button('_', "data/Glasses.png"));
  buttons.push_back(Button('+', "data/Filler.png"));
  buttons.push_back(Button('+', "data/Face.png"));
  buttons.push_back(Button('+', "data/Heart.png"));

  extras.push_back(Button('a', "data/charging.png")); // referenced as a
  extras.push_back(Button('b', "data/newmatch.png"));
  extras.push_back(Button('c', "data/cancel.png")); // referenced as c
  extras.push_back(Button('d', "data/noway.png"));
  extras.push_back(Button('e', "data/vs.png"));
  extras.push_back(Button('f', "data/icon.png"));
  extras.push_back(Button('g', "data/ok.png"));
  extras.push_back(Button('h', "data/ok2.png"));
  extras.push_back(Button('i', "data/shopclose.png"));
  extras.push_back(Button('j', "data/tomyzoo.png"));
  extras.push_back(Button('k', "data/face1.png"));
  extras.push_back(Button('l', "data/face2.png"));
  extras.push_back(Button('m', "data/start.png"));
  extras.push_back(Button('n', "data/face3.png"));
  extras.push_back(Button('p', "data/face4.png"));
  extras.push_back(Button('z', "data/back.png"));
}

void printZoo(const Zookeeper &r)
{
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++)
      printf("%c", r(y, x));
    printf("\n");
  }
}

int countZoo(const Zookeeper &r)
{
  int count = 0;
  for (int y = 0; y < 8; y++)
    for (int x = 0; x < 8; x++)
      if (r(y, x) != ' ')
	count++;
  
  return count;
}

bool compare(const Zookeeper &r, int y1, int x1, int dy, int dx) {
  int x2 = x1 + dx;
  int y2 = y1 + dy;
  if (y2 >= 8 || x2 >= 8 || y2 < 0 || x2 < 0)
    return false;

  return r(y1, x1) == r(y2, x2);
}

int connect_monkey()
{
  // first kill the old monkey
  FILE *ps = popen("adb shell ps", "r");
  char buffer[300];
  pid_t monkey = -1;
  while (fgets(buffer, sizeof(buffer) - 1, ps)) {
    if (strstr(buffer, "com.android.commands.monkey")) {
      printf("PS: %s", buffer);
      char *p = buffer + 7; // ignore command
      while (*p && !(*p >= '0' && *p <= '9')) p++;
      monkey = atoi(p);
    }
  }
  pclose(ps);
  if (monkey > 0) {
    sprintf(buffer, "adb shell kill %d", monkey);
    system(buffer);
  }

  sprintf(buffer, "adb shell monkey --port 1090 &");
  system(buffer);

  sprintf(buffer, "adb forward tcp:1090 tcp:1090");
  if (system(buffer)) {
    fprintf(stderr, "failed to port forward\n");
    exit(1);
  }
  
  //usleep(500000);
  sleep(1);
  
  int sock;
  struct sockaddr_in server;
  
  //Create socket
  sock = socket(AF_INET , SOCK_STREAM , 0);
  if (sock == -1)
    printf("Could not create socket");
    
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_family = AF_INET;
  server.sin_port = htons( 1090 );
  
  //Connect to remote server
  if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0) {
    perror("connect failed. Error");
    exit(1);
  }
  
  puts("Connected monkey\n");
  return sock;
}

void send_monkey_cmd(const string &message) {
  //Send some data
  if( send(monkey_socket , message.c_str(), message.length() , 0) < 0) {
    puts("Send failed");
    return;
  }

  char server_reply[2000];
  
  //Receive a reply from the server
  if( recv(monkey_socket , server_reply , 2000 , 0) < 0) {
    puts("recv failed");
    return;
  }
  if (server_reply[0] != 'O' || server_reply[1] != 'K')
    printf("Server: %s\n", server_reply);
}

void output_drag(int y1, int x1, int y2, int x2)
{
  y1 = (y1 * 24 + min_y + cut_y + 12) / scale;
  y2 = (y2 * 24 + min_y + cut_y + 12) / scale;
  
  x1 = (x1 * 24 + 1 + 12) / scale;
  x2 = (x2 * 24 + 1 + 12) / scale;
  
  char buffer[300];
  sprintf(buffer, "swipe %d %d %d %d", x1, y1, x2, y2);
  printf("%s\n", buffer);
  if (do_adb) {
    sprintf(buffer, "touch down %d %d\n", x1, y1);
    send_monkey_cmd(buffer);
    usleep(20000);
    sprintf(buffer, "touch move %d %d\n", x2, y2);
    send_monkey_cmd(buffer);
  }
}

bool check_drag(const Zookeeper &r, int y, int x, int dy, int dx, string comment, int my, int mx) {
  if (compare(r, y, x, dy, dx)) {
    //printf("move %c(%d:%d) %s\n", r(y, x), y+dy, x+dx, comment.c_str());
    output_drag( y+dy, x+dx, y+dy+my, x+dx+mx );
    return true;
  }
  return false;
}

void monkey_press(int x, int y)
{
  char buffer[300];
  x /= scale;
  y /= scale;
  printf("touch %d %d\n", x, y);
  sprintf(buffer, "touch down %d %d\n", x, y);
  send_monkey_cmd(buffer);
  usleep(10000);
  sprintf(buffer, "touch up %d %d\n", x, y);
  send_monkey_cmd(buffer);
}


void saveScreen(const string &prefix) 
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  char buffer[300];
  sprintf(buffer, "%s-%ld.%ld.png", prefix.c_str(), tv.tv_sec, tv.tv_usec);
  imwrite(buffer, frame);
}

void checkMoves(const Zookeeper &r)
{
  for (int y = 7; y >= 0; y--) {
    for (int x = 0; x < 8; x++) {
      if (r(y, x) == ' ')
	continue;

      if (r(y, x) == '+') { // all good things
	monkey_press(x * 24 + 12, y * 24 + min_y + cut_y + 12);
	return;
      }
      if (compare(r, y, x, 0, 1)) {
	if (check_drag(r, y, x, -1, -1, "down 1", +1, 0)) return;
	if (check_drag(r, y, x,  1, -1, "up 1", -1, 0)) return;
	if (check_drag(r, y, x, -1, +2, "down 1", +1, 0)) return;
	if (check_drag(r, y, x, +1, +2, "up 1", -1, 0)) return;
	if (check_drag(r, y, x,  0, +3, "left 2",  0, -1)) return;
	if (check_drag(r, y, x, 0, -2, "right 2", 0, 1)) return;
      }
      if (compare(r, y, x, 1, 0)) {
	if (check_drag(r, y, x, +2, -1, "right 1", 0, +1)) return;
	if (check_drag(r, y, x, +2, +1, "left 1",  0, -1)) return;
	if (check_drag(r, y, x, -1, -1, "right 3", 0, +1)) return;
	if (check_drag(r, y, x, -1, +1, "left 3",  0, -1)) return;
	if (check_drag(r, y, x, -2, 0, "down 3", +1, 0)) return;
	if (check_drag(r, y, x, 3, 0, "up 3", -1, 0)) return;
      }
      if (compare(r, y, x, 0, 2)) {
	if (check_drag(r, y, x, -1, 1, "down 4", +1, 0)) return;
	if (check_drag(r, y, x, 1, 1, "up 4", -1, 0)) return;
      }
      if (compare(r, y, x, 2, 0)) {
	if (check_drag(r, y, x, 1, -1, "right 4", 0, 1)) return;
	if (check_drag(r, y, x, 1, 1, "left 4", 0, -1)) return;
      }
    }
  }
  // last resort: glasses
  for (int y = 7; y >= 0; y--) {
    for (int x = 0; x < 8; x++) {
      if (r(y, x) == '_') { // all good things
	monkey_press(x * 24 + 12, y * 24 + min_y + cut_y + 12);
	return;
      }
    }
  }

}

Zookeeper catcher(Mat &frame)
{
  Mat resized;
  scale = 192. / frame.cols;
  resize(frame, resized, Size(0, 0), scale, scale);

  cut_y = (resized.rows - 320) / 2;
  resized = Mat(resized, Rect(0, cut_y, 192, 320));
  frame = resized;
  cvtColor(frame, frame, CV_BGR2GRAY );
  frame.convertTo(frame, CV_8UC1);

  Zookeeper res;

  int part_y = 24;
  int part_x = 24;
  int min_x = 0;
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      std::vector<Button>::const_iterator it = buttons.begin();
      Mat part(frame, Rect(24 * x + min_x, 24 * y + min_y, part_x, part_y));
      double maxmse = HUGE_VAL;
      char maxletter = ' ';
      for (; it != buttons.end(); ++it) {
	double mse = enhancedMSE(part, it->eightbit );
	if (mse < maxmse) {
	  maxmse = mse;
	  maxletter = it->letter;
	}
	if (maxmse < 500) // we're fine
	  break;
      }
      if (maxmse > 2000) {
	if (!do_adb) {
	  Mat region(frame, Rect(24 * x + min_x, 24 * y + min_y, part_x, part_y));
	  char buffer[300];
	  sprintf(buffer, "region-%06d-%c.png", int(maxmse), maxletter);
	  imwrite(buffer, region);
	}
	maxletter = ' ';
      }
      res(y, x) = maxletter;
    }
  }
  return res;
}

pid_t adb_pid = 0;

void setupCap(VideoCapture &cap) {
  unlink("record_mp4.fifo");
  mkfifo("record_mp4.fifo", 0600);
  if (adb_pid)
    kill(adb_pid, SIGTERM);
  adb_pid = fork();
  if (!adb_pid) {
    int record_fd = open("record_mp4.fifo", O_WRONLY);
    dup2(record_fd, STDOUT_FILENO);
    execlp("adb", "adb", "shell", "screenrecord", "--output-format=h264", "-", (char*)0);
    fprintf(stderr, "couldn't exec adb\n");
    exit(1);
  }
  cap.open("record_mp4.fifo");
  if(!cap.isOpened())  // check if we succeeded
    exit(1);
}

pid_t catcher_pid = 0;

void reexec(int _sleep = 0)
{
  kill(adb_pid, SIGTERM);
  waitpid(adb_pid, NULL, 0);
  if (catcher_pid)
    waitpid(catcher_pid, NULL, 0);
  sleep(_sleep);
  printf("reexec\n");
  execlp("./record", "record", (char*)0);
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

Button find_extra(const Mat & frame)
{
  std::vector<Button>::iterator it = extras.begin();
  for (; it != extras.end(); ++it) {
    Point2i button = image_search(frame, *it);
    if (button.x) {
      it->x = button.x;
      it->y = button.y;
      return *it;
    }
  }
  return Button();
}

int main(int argc, char**argv)
{
  init_buttons();

  if (argc > 1) {
    do_adb = 0;
    frame = imread(argv[1]);
    Zookeeper res = catcher(frame);
    int count = countZoo(res);
    if (count) {
      printZoo(res);
      checkMoves(res);
      exit(0);
    } else {
      Button button = find_extra(frame);
      cout << button.letter << endl;
      exit(1);
    }
  }

  monkey_socket = connect_monkey();
  
  signal(SIGINT, sig_handler);

#if 1
  Scalar m = mean(screencap());
  if (m[0] + m[1] < 10) { // looks black
    //printf("power!\n");
    //system("adb shell input keyevent 26");
    sleep(1);
  }
#endif
  
  VideoCapture cap;
  
  //sleep(2);
  if (screenrecord)
    setupCap(cap);

  int status;
  
  struct timeval tv, oldtv;
  gettimeofday(&oldtv, 0);

  //namedWindow("edges", WINDOW_AUTOSIZE);
  while (do_loop) {
    if (screenrecord) {
      if (!cap.read(frame)) {
	// we need to kill and leave - reopening video crashes opencv ;(
	reexec();
      }
    } else {
      frame = screencap();
    }
    
    gettimeofday(&tv, 0);
    int diff = (tv.tv_sec - oldtv.tv_sec) * 1000 + (tv.tv_usec - oldtv.tv_usec) / 1000;
    if (catcher_pid && waitpid(catcher_pid, &status, WNOHANG) == catcher_pid) {
      status = WEXITSTATUS(status);
      printf("catcher_pid finished: %d %d\n", status, diff);
      catcher_pid = 0;
      if (status == 12)
	reexec();
    }
    
    if (!catcher_pid && diff > HUMAN_LOOK_ALIKE) {
      catcher_pid = fork();
      if (!catcher_pid) {
	Zookeeper res = catcher(frame);
	int count = countZoo(res);
	if (count) {
#if 0
	  if (count != 64) {
	    char buffer[40];
	    sprintf(buffer, "missing-%d", count);
	    saveScreen(buffer);
	  }
#endif
	  //printZoo(count, res);
	  checkMoves(res);
	} else {
	  Button button = find_extra(frame);
	  if (button.x) {
	    if (button.letter == 'a') {
	      system("adb shell input touchscreen swipe 450 1000 450 300 500");
	    } else {
	      monkey_press(button.x + button.cols / 2, button.y + button.rows / 2);
	    }
	    if (button.letter == 'c') {
	      // power off
	      system("adb shell input keyevent 26");
	      kill(adb_pid, SIGTERM);
	      sleep(100);
	      exit(12);
	    } else {
	      sleep(1);
	    }
	  } else {
	    //saveScreen("nobutton");
	  }
	}
	exit(0);
      }
      oldtv = tv;
      //imshow("edges", frame);
      //if (waitKey(2) > 0) break;
    }
    
  }
  printf("kill\n");
  kill(adb_pid, SIGTERM);
  wait(&status);
  exit(0); // avoid the crashing destructor ;(
  // the camera will be deinitialized automatically in VideoCapture destructor
  return 0;
}