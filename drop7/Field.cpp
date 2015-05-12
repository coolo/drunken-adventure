#include "Field.h"
#include <cstring>
#include <cassert>
#include <iostream>

using namespace std;

Field::Field() {
  memset(data, ' ', 49);
}

Field Field::from_string(const char *text)
{
  assert(strlen(text) == 7 * 8);
  Field n;
  for (int y = 0; y < 7; y++) {
    for (int x = 0; x < 7; x++) {
      n.set(y, x, *text);
      text++;
    }
    text++; // skip newline
  }
  return n;
}

char Field::at(int y, int x) const
{
  if (y < 0 || y >= 7 || x < 0 || x >= 7)
    return ' ';
   
  return data[y*7+x];
}

void Field::set(int y, int x, char c)
{
  assert(y >= 0 && y < 7);
  assert(x >= 0 && x < 7);

  assert(c == ' ' || (c >= '1' && c <= '7'));
  data[y*7+x] = c;
}

string Field::to_string() const
{
  string ret;
  for (int y = 0; y < 7; y++)
    {
      for (int x = 0; x < 7; x++)
	ret += this->at(y, x);
      ret += "\n";
    }
  return ret;
}

Field Field::drop(char c, int col) const {
  assert(this->at(0, col - 1) == ' '); // not yet lost
  
  Field ret = *this;
  int y = 0;
  
  while (ret.at(y, col - 1) == ' ' && y < 6)
    y++;
  ret.set(y, col - 1, c);

  ret.blink();
  
  return ret;
}

bool Field::check_row(int y, int start_x, char c, char *marked) {

  //cerr << "check_row " << y << " " << start_x << " " << (char)(c + '0') << endl;

  if (this->at(y, start_x - 1) != ' ')
    return false;
  if (this->at(y, c + start_x) != ' ')
    return false;
  
  for (int i = 0; i < c; ++i) {
    if (this->at(y, i + start_x) == ' ')
      return false;
  }
  
  for (int x = start_x; x < start_x + c ; ++x) {
    if (this->at(y, x) == c + '0') {
      cerr << this->to_string();
      cerr << "found one " << y << " " << x <<  " " << (char)(c + '0') << endl;
      marked[y * 7 + x] = 1;
    }
  }
    
  return true;
}
	       
bool Field::blink() {
  bool foundone = false;

  char marked[49];
  memset(marked, 0, 49);
  
  for (int y = 0; y < 7; y++) 
    for (char c = 1; c <= 7; c++)
      for (int x = 0; x < 7 - (c - 1); x++)
	foundone = check_row(y, x, c, marked) || foundone;
  
  return foundone;
}
