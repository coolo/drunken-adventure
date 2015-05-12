#include "Field.h"

int main()
{
  const char *init =
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "       \n"
    "2  3 47\n";
  
  Field f = Field::from_string(init);
  f.drop('1', 2);
}
