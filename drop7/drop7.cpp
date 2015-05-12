#include "Field.h"
#include <iostream>

using namespace std;

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
  
  Field f = Field::from_string(init);
  for (int x = 0; x < 7; x++) {
    Field droped = f.drop('1', x + 1);
    //cerr << droped.to_string();
    //cerr << endl << endl;
    
    cerr << x << " " << droped.recursive_rating(3) << endl;
  }
}
