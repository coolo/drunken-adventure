#include "Field.h"
#include <cstring>
#include <cassert>
#include <iostream>

using namespace std;

Field::Field() {
  memset(data, ' ', 49);
}

Field::Field(const Field &f) {
  memcpy(data, f.data, 49);
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

  assert(c == ' ' || (c >= '0' && c <= '7') || c == 'A' || c == 'B');
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
  
  while (ret.at(y + 1, col - 1) == ' ' && y < 6)
    y++;
  ret.set(y, col - 1, c);

  ret.blink();
  
  return ret;
}

bool Field::check_row(int y, int start_x, char c, char *marked) {

  if (this->at(y, start_x - 1) != ' ')
    return false;
  if (this->at(y, c + start_x) != ' ')
    return false;
  
  for (int i = 0; i < c; ++i) {
    if (this->at(y, i + start_x) == ' ')
      return false;
  }

  bool foundone = false;
  for (int x = start_x; x < start_x + c ; ++x) {
    if (this->at(y, x) == c + '0') {
      marked[y * 7 + x] = 1;
      foundone = true;
    }
  }
    
  return foundone;
}

bool Field::check_col(int start_y, int x, char c, char *marked) {

  if (this->at(start_y - 1, x) != ' ')
    return false;
  if (this->at(c + start_y, x) != ' ')
    return false;
  
  for (int i = 0; i < c; ++i) {
    if (this->at(i + start_y, x) == ' ')
      return false;
  }

  bool foundone = false;
  for (int y = start_y; y < start_y + c ; ++y) {
    if (this->at(y, x) == c + '0') {
      marked[y * 7 + x] = 1;
      foundone = true;
    }
  }
    
  return foundone;
}

void Field::markturn(int y, int x, char *marked) {
  if (at(y, x) == 'B' || at(y, x) == 'A')
    marked[y * 7 + x] = 1;
}

bool Field::gravitate() {
  bool moved = false;
  for (int x = 0; x < 7; ++x) {
    for (int y = 5; y >= 0; y--) {
      if (at(y, x) != ' '&& at(y + 1, x) == ' ')
	{
	  set(y + 1, x, at(y, x));
	  set(y, x, ' ');
	  y = 6;
	  moved = true;
	}
    }
  }
  return moved;
}

bool Field::blink() {
  bool foundone = false;

  char marked[49];
  memset(marked, 0, 49);
  
  for (int y = 0; y < 7; y++) 
    for (char c = 1; c <= 7; c++)
      for (int x = 0; x < 7 - (c - 1); x++)
	foundone = check_row(y, x, c, marked) || foundone;
  
  for (int x = 0; x < 7; x++) 
    for (char c = 1; c <= 7; c++)
      for (int y = 0; y < 7 - (c - 1); y++)
	foundone = check_col(y, x, c, marked) || foundone;

  char turn[49];
  memset(turn, 0, 49);

  if (foundone) {
    for (int y = 0; y < 7; y++) 
      for (int x = 0; x < 7; x++)
	if (marked[y * 7 + x]) {
	  set(y, x, ' ');
	  markturn(y - 1, x, turn);
	  markturn(y + 1, x, turn);
	  markturn(y, x - 1, turn);
	  markturn(y, x + 1, turn);
	}
    for (int y = 0; y < 7; y++) 
      for (int x = 0; x < 7; x++)
	if (turn[y * 7 + x]) {
	  if (at(y, x) == 'B')
	    set(y, x, 'A');
	  else
	    set(y, x, '0');
	}
    gravitate();
  }
  return foundone;
}
