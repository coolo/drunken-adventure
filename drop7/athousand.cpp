#include "Field.h"
#include <iostream>
#include <string.h>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <map>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

void random_seed() {
  int fd = open("/dev/urandom", O_RDONLY);
  unsigned int buffer[1];
  read(fd, buffer, sizeof(unsigned int));
  close(fd);
  srand(buffer[0]);
}

void do_moves(Field &f, int count = 5) {
  while (count-- && f.elements() < 49) {
    int col = 0;
    while (!col || !f.free_column(col)) {
      col = rand() % 7 + 1;
    }
    Stone n = f.next_stone();
    f = f.drop(n, col);
    while (f.blink());
  }
}

long random_score(const Field &_f, int count = 500) {
  long score = 0;
  for (int i = 0; i < count; ++i) {
    Field f = _f;
    f.set_random(0);
    do_moves(f, 20);
    //cout << f.to_string() << f.score() << endl;
    score += (f.score() - _f.score());
  }
  return score / count;
}

int main(int argc, char **argv)
{
  random_seed();

  Field f;
  f.set_random();
  while (f.blink());
  
  while (true) {
    cout << f.to_string();
    Stone n = f.next_stone();
    int best_col = 0;
    int bscore = 0;
    for (int col = 1; col < 8; col++) {
      Field f2 = f.drop(n, col);
      while (f2.blink());
      long rscore = random_score(f2) + (f2.score() - f.score());
      cout << n.to_char() << " " << col << " " << f2.score() << " " << rscore << endl;
      if (bscore < rscore) {
	bscore = rscore;
	best_col = col;
      }
    }
    f = f.drop(n, best_col);
    while (f.blink());
    cout << "moved to " << f.score() << f.to_string();
  }
  
  return 0;
}
