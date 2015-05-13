#include "Field.h"
#include <iostream>
#include <limits>
#include <stdlib.h>

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

int main()
{
  const char *init =
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "545  5 \n"
    "5A6  15\n"
    "BBB46BB\n";

  srand(time(0));
  
  Field f = Field::from_string(init);

  while (1) {
    int choice = rand() % 8;
    if (choice == 0) {
      turn(f, 'B');
    } else {
      turn(f, '0' + choice);
    }
  }

}
