#include "opencv2/opencv.hpp"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <arpa/inet.h> //inet_addr
#include <string.h> //strlen
#include <sys/socket.h> //socket

#include <iostream>
#include <list>

using namespace cv;
using namespace std;

const int HUMAN_LOOK_ALIKE = 730;

int sleep_time = HUMAN_LOOK_ALIKE;

int do_loop = 1;
int do_adb = 1;

int monkey_socket = -1;

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

    Button()
    {
        x = y = 0;
        letter = ' ';
    }

    Button(char _letter, const char* filename)
    {
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

    Move(int _weight, int _x, int _y, int _dx = 0, int _dy = 0)
    {
        weight = _weight;
        x = _x;
        y = _y;
        dx = _dx;
        dy = _dy;
    }
};

void sig_handler(int signum)
{
    fprintf(stderr, "signhandler %d\n", signum);
    do_loop = 0;
}

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
    } else {
        printf("stalled alarm\n");
        system("adb reboot");
        sleep(300);
    }
    exit(1);
}

/* the purpose of this function is to calculate the error between two images
  (scene area and object) ignoring slight colour changes */
double enhancedMSE(const Mat& I1, const Mat& I2)
{
    assert(I1.channels() == 1);
    assert(I2.channels() == 1);
    assert(I1.rows == I2.rows);
    assert(I1.cols == I2.cols);

    double sse = 0;

    for (int j = 0; j < I1.rows; j++) {
        // get the address of row j
        const uchar* I1_data = I1.ptr<const uchar>(j);
        const uchar* I2_data = I2.ptr<const uchar>(j);

        for (int i = 0; i < I1.cols; i++) {
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
Point2i image_search(const Mat& scene, const Button& button, double mselimit = 1000)
{

    const Mat object = button.eightbit;

    int result_width = scene.cols - object.cols + 1;
    int result_height = scene.rows - object.rows + 1;
    Mat result = Mat::zeros(result_height, result_width, CV_32FC1);

    // Perform the matching. Info about algorithm:
    // http://docs.opencv.org/trunk/doc/tutorials/imgproc/histograms/template_matching/template_matching.html
    // http://docs.opencv.org/modules/imgproc/doc/object_detection.html
    matchTemplate(scene, object, result, CV_TM_SQDIFF_NORMED);

    // Localizing the best match with minMaxLoc
    double minval, maxval;
    Point minloc, maxloc;
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
    return Point(0, 0);
}

vector<Button> buttons;
typedef Matx<char, 8, 8> Zookeeper;

vector<Button> extras;

void init_buttons()
{
    extras.push_back(Button('a', "data/charging.png")); // referenced as a
    extras.push_back(Button('e', "data/app.png")); // referenced as e
    extras.push_back(Button('A', "data/closenew.png"));
    extras.push_back(Button('B', "data/closenew2.png"));
    extras.push_back(Button('F', "data/closenew3.png"));
    extras.push_back(Button('E', "data/closedialog.png"));
    extras.push_back(Button('C', "data/coin.png"));
    //extras.push_back(Button('p', "data/portal-big.png"));
    //extras.push_back(Button('P', "data/pause.png"));
}

int connect_monkey()
{
    // first kill the old monkey
    FILE* ps = popen("adb shell ps", "r");
    char buffer[300];
    pid_t monkey = -1;
    while (fgets(buffer, sizeof(buffer) - 1, ps)) {
        if (strstr(buffer, "com.android.commands.monkey")) {
            char* p = buffer + 7; // ignore command
            while (*p && !(*p >= '0' && *p <= '9'))
                p++;
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
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        printf("Could not create socket");

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(1090);

    //Connect to remote server
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect failed. Error");
        exit(1);
    }

    return sock;
}

void send_monkey_cmd(const string& message)
{
    //Send some data
    if (send(monkey_socket, message.c_str(), message.length(), 0) < 0) {
        puts("Send failed");
        return;
    }

    char server_reply[2000];

    //Receive a reply from the server
    if (recv(monkey_socket, server_reply, 2000, 0) < 0) {
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

Zookeeper calculate_new_zoo(const Zookeeper& r, int y, int x, int dy, int dx)
{
    Zookeeper newz = r;
    char old = newz(y, x);
    newz(y, x) = newz(y + dy, x + dx);
    newz(y + dy, x + dx) = old;
    return newz;
}

Zookeeper blackoutZoo(const Zookeeper& _r)
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

void monkey_press(int x, int y, bool rotated = true)
{
    char buffer[300];
    x /= scale;
    y /= scale;
    printf("touch %d %d\n", x, y);
    if (rotated) {
        int s = x;
        x = y;
        y = 768 - s;
    }
    sprintf(buffer, "touch down %d %d\n", x, y);
    send_monkey_cmd(buffer);
    usleep(10000);
    sprintf(buffer, "touch up %d %d\n", x, y);
    send_monkey_cmd(buffer);
}

void saveScreen(const string& prefix)
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    char buffer[300];
    sprintf(buffer, "%s-%ld.%ld.png", prefix.c_str(), tv.tv_sec, tv.tv_usec);
    imwrite(buffer, frame);
    printf("saved %s\n", buffer);
}

Mat original;

void catcher(Mat& frame)
{
    //saveScreen("catcher");
    original = frame;
    Mat resized;
    scale = 192. / frame.cols;
    resize(frame, resized, Size(0, 0), scale, scale);

    cut_y = (resized.rows - 320) / 2;
    resized = Mat(resized, Rect(0, cut_y, 192, 320));
    frame = resized;
    cvtColor(frame, frame, CV_BGR2GRAY);
    frame.convertTo(frame, CV_8UC1);

    saveScreen("frame");
    //return res;
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

Mat screencap()
{
    struct timeval tv;
    FILE* adb = popen("adb shell screencap -p", "r");
    gettimeofday(&tv, 0);
    char fname[300];
    sprintf(fname, "current-%ld.%ld.png", tv.tv_sec, tv.tv_usec);
    FILE* png = fopen(fname, "w");
    char buffer[3000];
    size_t all = 0;
    for (;;) {
        size_t bytes = fread(buffer, 1, sizeof(buffer) - 1, adb);
        all += bytes;
        if (!bytes)
            break;
        // now make sure we read 0x0d0a completey
        if (buffer[bytes - 1] == 0xd) {
            buffer[bytes++] = fgetc(adb);
        }
        // undo EOL replacement
        for (unsigned int i = 0; i < bytes; ++i) {
            if (buffer[i] == 0xd && buffer[i + 1] == 0xa) {
                buffer[i] = 0xa;
                for (unsigned int j = i + 1; j < bytes - 1; ++j)
                    buffer[j] = buffer[j + 1];
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

Button find_extra(const Mat& frame)
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

bool check_button(const char* filename)
{
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

int main(int argc, char** argv)
{
    init_buttons();

    if (argc > 1) {
        do_adb = 0;
        frame = imread(argv[1]);
        catcher(frame);

        Button button = find_extra(frame);
        cout << button.letter << endl;
        exit(1);
    }

    monkey_socket = connect_monkey();
    sleep(1);

    signal(SIGINT, sig_handler);
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGALRM, sigalarm_handler);

#if 1
    Scalar m = mean(screencap());
    if (m[0] + m[1] < 10) { // looks black
        printf("power!\n");
        system("adb shell input keyevent 26");
        sleep(1);
    }
#endif

    int status;

    struct timeval tv, oldtv;
    gettimeofday(&oldtv, 0);

    //namedWindow("edges", WINDOW_AUTOSIZE);
    while (do_loop) {
        frame = screencap();

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
                for (; it != extras.end(); ++it) {
                    if (it->letter == 'e') {
                        extras.erase(it);
                        break;
                    }
                }
                extras.push_back(Button('v', "data/vs.png"));
            }
        }

        fprintf(stderr, "catcher %ld\n", catcher_pid);
        if (!catcher_pid && diff > sleep_time) {
            catcher_pid = fork();
            if (!catcher_pid) {
                //saveScreen("movie");
                catcher(frame);

                //saveScreen("extras");
                Button button = find_extra(frame);
                fprintf(stderr, "button %c\n", button.letter);
                if (button.x) {
                    if (button.letter == 'a') {
                        system("adb shell input touchscreen swipe 450 1000 450 300 500");
                        sleep(3);
                    } else if (button.letter == 'e') {
                        monkey_press(button.x + 10, button.y + 10, false);
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
                    int rx = rand() % 500 - 250;
                    int ry = rand() % 600 - 300;
                    char buffer[300];
                    sprintf(buffer, "adb shell input touchscreen swipe 450 700 %d %d 200", 450 + rx, 700 + ry);
                    system(buffer);
                    fprintf(stderr, "do %d %d %s\n", rx, ry, buffer);
                    Scalar m = mean(frame);
                    if (m[0] + m[1] < 10) { // looks black
                        printf("black\n");
                        exit(13);
                    }
                    // saveScreen("nobutton");
                }

                //exit(0);
            }
            oldtv = tv;
            //imshow("edges", frame);
            //if (waitKey(2) > 0)
            //    break;
        }
    }
    printf("kill\n");
    kill(adb_pid, SIGTERM);
    wait(&status);
    exit(0); // avoid the crashing destructor ;(
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
