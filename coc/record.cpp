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

const int HUMAN_LOOK_ALIKE = 730;

int sleep_time = HUMAN_LOOK_ALIKE;

int do_loop = 1;
int do_adb = 1;

int monkey_socket = -1;
bool rotated = false;
bool ingame = false;

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

struct Move {
  int x, y;
  int dx, dy;
  int weight;

  Move() {}
  
  Move(int _weight, int _x, int _y, int _dx = 0, int _dy = 0) {
    weight = _weight;
    x = _x;
    y = _y;
    dx = _dx;
    dy = _dy;
  }
};

void sigusr1_handler(int signum)
{
  sleep_time = 300;
}

void sigalarm_handler(int signum)
{
  printf("alarm clock rang\n");
  Scalar m = mean(frame);
  if (m[0] + m[1] < 10) { // looks black
      printf("black alarm\n");
  } 
  exit(1);
}

/* the purpose of this function is to calculate the error between two images
  (scene area and object) ignoring slight colour changes */
double enhancedMSE(const Mat& I1, const Mat& I2, int limit = -1) {
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
	  uchar t1 = I1_data[i];
	  if (limit > 0 && t1 < limit)
	    t1 = 0;
	  uchar t2 = I2_data[i];
	  if (limit > 0 && t2 < limit)
	    t2 = 0;
	  double diff = abs(t1 - t2);
	  sse += diff * diff;
        }
    }
  
  double total = I1.total();
  return sse / total;
}

/* we find the object in the scene and return the x,y and the error of the match */
Point2i image_search(const Mat &scene, const Button &button, double mselimit = 5000) {

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
  if (!do_adb) {
    char fname[300];
    sprintf(fname, "result-%c.png", button.letter);
    imwrite(fname, test);
  }
  //printf("MSE %lf\n", mse);
  if (mse < mselimit) 
    return minloc;
  return Point(0,0);
}

vector<Button> buttons;
typedef Matx<char, 8, 8> Zookeeper;

vector<Button> extras;

void init_buttons() {
}

int connect_monkey()
{
  // first kill the old monkey
  FILE *ps = popen("adb shell ps", "r");
  char buffer[300];
  pid_t monkey = -1;
  while (fgets(buffer, sizeof(buffer) - 1, ps)) {
    if (strstr(buffer, "com.android.commands.monkey")) {
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

Zookeeper calculate_new_zoo(const Zookeeper &r, int y, int x, int dy, int dx) {
  Zookeeper newz = r;
  char old = newz(y, x);
  newz(y, x) = newz(y + dy, x + dx);
  newz(y + dy, x + dx) = old;
  return newz;
}


Zookeeper blackoutZoo(const Zookeeper &_r)
{
  Zookeeper r = _r;
  for (int y = 7; y >= 0; y--) {
    for (int x = 0; x < 8; x++) {
      if (r(y, x) != ' ')
	continue;
      for (int i = y; i >= 0; i--) {
	r(i, x) = ' ';
      }
    }
  }
  return r;
}

void monkey_press(int x, int y)
{
  char buffer[300];
  x /= scale;
  y /= scale;
  if (rotated && !ingame) {
	  int z = frame.rows - y;
	  y = x;
	  x = z;
  }
  printf("touch %d %d %d - %dx%d\n", rotated, x, y, frame.cols, frame.rows);
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
  printf("saved %s\n", buffer);
}

int countAt(Mat &frame, int y)
{
  int count = 0;
  int x = 64;
  for (int z = 0; z < 6;) {
    int best_mse = 900000;
    int best_dig = -1;
    int best_dig_x = 0;
    for (int i = 0; i < 10; i++) {
      char buffer[30];
      sprintf(buffer, "data/%d.png", i);
      Mat p = imread(buffer, CV_LOAD_IMAGE_GRAYSCALE);
      Mat eins(frame, Rect(x, y, p.cols, p.rows));
      sprintf(buffer, "out-%d-%d.png", x, i);
      //imwrite(buffer, eins);
      int mse = enhancedMSE(eins, p, 200);
      //cout << i << " " << mse << endl;
      if (best_mse > mse) {
	best_mse = mse;
	best_dig = i;
	best_dig_x = eins.cols;
      }
    }
    if (best_mse > 3500) {
      //cout << "eins at++ " << best_dig << " " << x << " " << best_mse << endl;
      x++;
    } else {
      count *= 10;
      count += best_dig;
      
      //cout << "eins at " << best_dig << " " << x << " " << best_mse << " " << count << endl;
      x += best_dig_x;
      z++;
    }
    if (x > 180)
      break;
  }

  return count;
}

Mat original;

Zookeeper catcher(Mat &frame)
{
  //saveScreen("catcher");
  original = frame;
  Mat resized;
  scale = 1; // 384. / frame.cols;
  if (scale != 1)
    resize(frame, resized, Size(0, 0), scale, scale);
  else
    resized = frame;
  
  if (resized.rows > resized.cols) {
    transpose(resized, frame);
    flip(frame, frame, 0);
    rotated = true;
  } else {
    transpose(frame, resized);
    frame = resized;
  }
      
  cut_y = (frame.rows - 0) / 2;
  //resized = Mat(resized, Rect(0, cut_y, 384, 320));
 // frame = resized;
  cvtColor(frame, frame, CV_BGR2GRAY );
  frame.convertTo(frame, CV_8UC1);

  saveScreen("frame");
  Zookeeper res;

  int delta = 0;
    
  int first = countAt(frame, 90 + delta);
    
  if (!first) {
    first = countAt(frame, 91);
    delta++;
  }
  int second = countAt(frame, 128 + delta);
  int third = countAt(frame, 167 + delta);

  cout << "found: " << first << " " << second << " " << third << endl;
  
  if (first > 0 && (first < 200000 || second < 200000)) {
    Button t('a', "data/weiter.png");
    Point2i button = image_search(frame, t);
    cout << "close " << button.x << endl;
    if (button.x)
      monkey_press(button.x + t.cols / 2, button.y + t.rows / 2);
  }
  
  return res;
}

pid_t adb_pid = 0;

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

bool check_button(const char *filename) {
  Button t('a', filename);
  printf("image_search %s\n", filename);
  Point2i button = image_search(frame, t, 2000);
  if (button.x) {
    printf("check_button -> press %s\n", filename);
    monkey_press(button.x + t.cols / 2, button.y + t.rows / 2);
    sleep(1);
    return true;
  }
  return false;
}

int on_screen(const char *filename) {
   Button t('a', filename);
   Point2i button = image_search(frame, t, 1000);
   printf("on_screen %s: %dx%d\n", filename, button.x, button.y);
   return button.x;
}

bool check_reload(const char *filename) {
    Button t('a', filename); 
    Point2i button = image_search(frame, t, 1000);
    if (button.x) {
      printf("sleep 10 minutes\n");
      sleep(600); // 10 minutes
      monkey_press(button.x + t.cols / 2, button.y + t.rows / 2);
      return true;
    }
    return false;
}

bool fetchFrame() {
	  frame = screencap();
	      if (!frame.cols)
		            return false;

	          Mat resized = frame;
		      if (resized.rows > resized.cols) {
			            transpose(resized, frame);
				          flip(frame, frame, 0);
					        rotated = true;
						    } else {
							          transpose(frame, resized);
								        frame = resized;
									    }

		          cvtColor(frame, frame, CV_BGR2GRAY );
			      frame.convertTo(frame, CV_8UC1);

			          saveScreen("frame");
				  return true;

}

int main(int argc, char**argv)
{
  init_buttons();

  if (argc > 1) {
    do_adb = 0;
    frame = imread(argv[1]);
    catcher(frame);
    return 0;
  }

  monkey_socket = connect_monkey();
  
  signal(SIGUSR1, sigusr1_handler);
  signal(SIGALRM, sigalarm_handler);

  
  
#if 0
  Scalar m = mean(screencap());
  if (m[0] + m[1] < 10) { // looks black
    printf("power!\n");
    //system("adb shell input keyevent 26");
    sleep(1);
  }
#endif
  
  int status;
  
  struct timeval tv, oldtv;
  gettimeofday(&oldtv, 0);

  /*
  output_drag(100, 100, 550, 550);
  sleep(1);
  output_drag(100, 100, 550, 550);
  sleep(1);
  */

  int moved = 0;
  int direction = -1;
  
  fetchFrame();

  if (on_screen("data/lockscreen.png")) {
  }

  while (check_button("data/icon.png")) {
	  sleep(1);
	  fetchFrame();
  }
  ingame = true;
  check_button("data/okay.png");

  while (true) {
    sleep(1);

    if (!fetchFrame())
	    continue;

    if (check_reload("data/reload.png"))
	    continue;
    if (check_button("data/tryagain.png")) {
	    continue;
    }

    if (on_screen("data/attack.png")) {

    Button t('a', "data/rathaus4.png");
    Point2i rathaus = image_search(frame, t, 1000);
    cout << "rathaus " << rathaus.x << "x" << rathaus.y << endl;

    if (check_button("data/g.png"))
      continue;
    if (check_button("data/e.png"))
      continue;
    if (rathaus.x && (rathaus.x < 490 || rathaus.x > 510) ) {
         if (rathaus.y < 360 || rathaus.y > 400) {
		 output_drag(rathaus.y, rathaus.x, 380, rathaus.x);
	 } else if (rathaus.x > 750) {
		 output_drag(rathaus.y, rathaus.x, rathaus.y, 700);
	 } else {
		 output_drag(rathaus.y, rathaus.x, rathaus.y, 500);
	 }
	 continue;
    }
    const int delta = 100;
    output_drag(380, 380, 380 + delta * direction, 380 + delta * direction);
    moved += delta;
    if (moved > 700) {
      direction *= -1;
      moved = 0;
    }
    }
  }
  
  while (do_loop) {
        
    gettimeofday(&tv, 0);
    int diff = (tv.tv_sec - oldtv.tv_sec) * 1000 + (tv.tv_usec - oldtv.tv_usec) / 1000;
    if (catcher_pid && waitpid(catcher_pid, &status, WNOHANG) == catcher_pid) {
      status = WEXITSTATUS(status);
      //printf("catcher_pid finished: %d %d\n", status, diff);
      catcher_pid = 0;
      if (status == 12) 
        sleep(100);
      if (status == 12 || status == 13)
	reexec();
      if (status == 14) {
	std::vector<Button>::iterator it = extras.begin();
	for (; it != extras.end(); ++it) 
	   if (it->letter == 'e') {
	      extras.erase(it);
	      break;
	   }
           extras.push_back(Button('v', "data/vs.png"));
      }
    }
    
    if (!catcher_pid && diff > sleep_time) {
      catcher_pid = fork();
      if (!catcher_pid) {
	//saveScreen("movie");
	catcher(frame);
	if (false) {
	  //saveScreen("extras");
	  Button button = find_extra(frame);
	  if (button.x) {
	    if (button.letter == 'a') {
	      system("adb shell input touchscreen swipe 450 1000 450 300 500");
	    } else if (button.letter == 'q') {
	      // restart the timers
              exit(13);
	    } else if (button.letter == 'b') {
              // if we see newbattle, we actually press back
	      Button t('a', "data/exitfight.png");
              Point2i button = image_search(frame, t);
	      if (button.x) 
	        monkey_press(button.x + t.cols / 2, button.y + t.rows / 2);
	    } else if (button.letter == 'n') {
	      if (!check_button("data/startbattle2.png"))
                 check_button("data/exitfight.png");
	      sleep(1);
	    } else if (button.letter == 'r') {
	      // score
	      frame = original;
	      saveScreen("score");
	      sleep(4);
	      exit(13);
	    } else {
	      printf("pressing button %c\n", button.letter);
	      monkey_press(button.x + button.cols / 2, button.y + button.rows / 2);
	      sleep(1);
	    }
	    if (button.letter == 'c') {
	      // power off
	      system("adb shell input keyevent 26");
	      kill(adb_pid, SIGTERM);
	      exit(12);
	    } else {
	      sleep(1);
	    }
	  } else {
	    Scalar m = mean(frame);
            if (m[0] + m[1] < 10) { // looks black
		printf("black\n");
		exit(13);
     	    }
	   // saveScreen("nobutton");
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
