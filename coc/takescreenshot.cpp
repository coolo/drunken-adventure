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

int do_loop = 1;
int do_adb = 1;

pid_t adb_pid;

Mat frame;

void sig_handler(int signum)
{
  do_loop = 0;
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

Mat original;

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
  execlp("./takescreenshot", "takescreenshot", (char*)0);
}

void sigalarm_handler(int signum)
{
  reexec();
}

int main(int argc, char**argv)
{
  signal(SIGINT, sig_handler);
  signal(SIGALRM, sigalarm_handler);

  VideoCapture cap;
  
  //sleep(2);
  setupCap(cap);

  int status;
  
  struct timeval oldtv;
  gettimeofday(&oldtv, 0);

  namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
  
  while (do_loop) {
    alarm(5); // make sure we don't block here
    if (!cap.read(frame)) {
      // we need to kill and leave - reopening video crashes opencv ;(
      reexec();
    }
    saveScreen("current");
	kill(adb_pid, SIGTERM);
    exit(0);
    //imshow( "Display window", frame );       
    waitKey(1);   
    alarm(0);
  }
    
  kill(adb_pid, SIGTERM);
  wait(&status);
  exit(0); // avoid the crashing destructor ;(
  // the camera will be deinitialized automatically in VideoCapture destructor
  return 0;
}
