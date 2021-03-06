#include "Field.h"
#include <cstring>
#include <cassert>
#include <iostream>
#include <cstdio>
#include <map>
#include <cstdlib>

using namespace std;

Stone::Stone() {
  state = Shown;
  value = 0;
}

Stone::Stone(const Stone &rhs) {
  state = rhs.state;
  value = rhs.value;
}

void Stone::set_B() {
  state = SuperHidden;
}
    
Stone::Stone(char c) {
  if (c == 'B') {
    state = SuperHidden;
  } else if (c == 'A') {
    state = Hidden;
  } else if (c == ' ') {
    value = 0;
    return;
  } else {
    state = Shown;
    value = c - '0';
    return;
  }
  // need to guess
  value = rand() % 7;
}

void Stone::pick_random() {
  value = rand() % 7 + 1;
  if (rand() % 7 < 2)
    state = SuperHidden;
  else
    state = Shown;
}

void Stone::turn() {
  if (state == Hidden)
    state = Shown;
  if (state == SuperHidden)
    state = Hidden;
}

char Stone::to_char() const {
  if (!value)
    return ' ';
  if (state == Shown)
    return value + '0';
  if (state == Hidden)
    return 'A';
  return 'B';
}

string Stone::to_string() const {
  if (!value)
    return " ";
  char buffer[3];
  sprintf(buffer, "%d", value);
  string ret = buffer;
  if (state == Shown)
    return ret;
  if (state == Hidden)
    return ret + "!";
  return ret + "?";
}

bool Stone::is_null() const {
  assert(value >= 0 && value < 8);
  return value == 0;
}

static map<string, double> hashes;

Field::Field() {
  m_score = 0;
  m_step = 0;
  m_level = 1;
  m_dots = 30;
}

Field::Field(const Field &f) {
  for (int i = 0; i < 49; i++)
    data[i] = f.data[i];
  m_score = f.m_score;
  m_step = f.m_step;
  m_level = f.m_level;
  m_dots = f.m_dots;
  line = f.line;
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
   
  return data[y*7+x].to_char();
}

Stone Field::next_stone()
{
  assert(!line.empty());
  Stone e = line.front();
  line.pop_front();
  return e;
}

Stone Field::stone(int y, int x) const
{
  if (y < 0 || y >= 7 || x < 0 || x >= 7)
    return Stone();
   
  return data[y*7+x];
}

void Field::set(int y, int x, char c)
{
  assert(y >= 0 && y < 7);
  assert(x >= 0 && x < 7);

  assert(c == ' ' || (c >= '0' && c <= '7') || c == 'A' || c == 'B');
  data[y*7+x] = Stone(c);
}

void Field::set(int y, int x, Stone e)
{
  assert(y >= 0 && y < 7);
  assert(x >= 0 && x < 7);

  data[y*7+x] = e;
}

string Field::to_string() const
{
  string ret = "\nX=======X\n";
  for (int y = 0; y < 7; y++)
    {
      ret += "|";
      for (int x = 0; x < 7; x++)
	ret += this->at(y, x);
      ret += "|\n";
    }
  ret += "X=======X\n";
  return ret;
}

Field Field::drop(char c, int col) const {
  assert(this->at(0, col - 1) == ' '); // not yet lost

  Field ret = *this;
  int y = 0;
    
  while (ret.at(y + 1, col - 1) == ' ' && y < 6)
    y++;
  ret.set(y, col - 1, c);
  ret.m_step = 0;
  ret.m_dots = m_dots - 1;

  return ret;
}

Field Field::drop(Stone e, int col) const {
  assert(this->stone(0, col - 1).is_null()); // not yet lost

  Field ret = *this;
  int y = 0;

  while (ret.stone(y + 1, col - 1).is_null() && y < 6)
    y++;
  ret.set(y, col - 1, e);
  ret.m_step = 0;
  ret.m_dots = m_dots - 1;

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
    marked[y * 7 + x]++;
}

bool Field::gravitate() {
  bool moved = false;
  for (int x = 0; x < 7; ++x) {
    for (int y = 5; y >= 0; y--) {
      if (at(y, x) != ' ' && at(y + 1, x) == ' ')
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

inline int calculate_score(unsigned int step) {
  const int scores[] = { 7, 39, 109, 224, 391, 617, 907, 1267, 1701,
			 2213, 2809, 3491, 4265, 5133, 6099, 7168, 8341,
			 9622, 11014, 12521, 14146, 15891, 17758,
			 19752, 21875, 24128, 26515, 29039, 31702,
			 34506};
  if (step >= sizeof(scores) / sizeof(int))
    return 0;
  return scores[step];
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
	  int delta = calculate_score(m_step);
	  if (!delta) {
	    cerr << "STEP " << m_step << endl << to_string();
	    assert(delta);
	  }
	  m_score += delta;
	  markturn(y - 1, x, turn);
	  markturn(y + 1, x, turn);
	  markturn(y, x - 1, turn);
	  markturn(y, x + 1, turn);
	}

    for (int y = 0; y < 7; y++) 
      for (int x = 0; x < 7; x++)
	while (turn[y * 7 + x]--)
	  data[y * 7 + x].turn();
    
    gravitate();
    m_step++;
  }
  if (!foundone) {
    if (!elements()) {
      m_score += 70000;
    }
    if (!m_dots) {
      add_B_row();
      m_level++;
      m_score += 7000;
      m_dots = 5 + max(26 - m_level, 0);
    }
  }
  return foundone;
}

int Field::elements() const {
  int r = 0;
  for (int y = 0; y < 7; y++) 
    for (int x = 0; x < 7; x++)
      if (!data[y*7+x].is_null())
	r++;
  return r;
}

double Field::rating() const {
  double count = 0;
  for (int y = 0; y < 7; y++) 
    for (int x = 0; x < 7; x++) {
      switch (at(y, x))
	{
	case ' ':
	  break;
	case 'B':
	  count += (28 - y) * (28 - y);
	  break;
	case 'A':
	  count += (16 - y) * (16 - y);
	  break;
	default:
	  int r = 14 - y - (at(y, x) - '0');
	  count += r * r;
	}
    }
  return count;
}

inline char maptochar(int i) {
  if (i == 8)
    return 'B';
  if (i == 7)
    return 'A';
  return '1' + i;
}

double Field::recursive_rating(int depth) const {
  double r = 0;

  if (!depth) {
    return rating();
  }
  depth--;

  char buffer[65];
  sprintf(buffer, "%d-%s", depth, to_string().c_str());

  map<string,double>::const_iterator it = hashes.find(buffer);
  if (it != hashes.end())
    return it->second;
  
  for (int y = 0; y < 7; y++)
    for (int x = 0; x < 7; x++)
      if (at(y, x) == '0') {
	Field f = *this;
	for (int c = 1; c < 8; c++) {
	  //cerr << "replacing 0 with " << c << endl;
	  f.set(y, x, c + '0');
	  r += f.recursive_rating(depth);
	}
	return r / 7;
      }

  int count = 0;
  for (int x1 = 0; x1 < 7; x1++) {
    if (at(0, x1) != ' ')
      continue;
    count++;
    double t = 0;
    for (int c = 0; c < 9; c++) {
      //cerr << depth << " drop " << map(c) << " " << x1 + 1 << endl;
      t += drop(maptochar(c), x1 + 1).recursive_rating(depth);
    }
    r += t / 9;
  }
  r = r / count;

  hashes[buffer] = r;
  //cerr << buffer << " " << hashes.size() << endl;

  return r;
}

void Field::finalize_random() {
  for (int y = 0; y < 7; y++)
    for (int x = 0; x < 7; x++)
      if (at(y, x) == '0') {
	char number = int(rand() % 7) + '1';
	set(y, x, number);
      }
}

bool Field::add_B_row() {
  for (int x = 0; x < 7; x++) {
    if (at(0, x) != ' ')
      return false; // lost
    for (int y = 0; y < 6; y++)
      set(y, x, at(y + 1, x));
    Stone b = next_stone();
    b.set_B();
    set(6, x, b);
  }
  //cerr << to_string();
  return true;
}

void Field::ann_input(float *a) const {
  for (int i = 0; i < 49; ++i) {
    for (int c = 0; c < 9; c++)
      a[i*9+c] = 0;
    char c = data[i].to_char();
    switch (c) {
    case ' ':
      break;
    case 'B':
      a[i*9+9] = 1;
      break;
    case 'A':
      a[i*9+8] = 1;
      break;
    default:
      a[i*9+c - '0'] = 1;
    }
  }
}

void Field::set_random(int elements) {
  line.clear();
  for (int i = 0; i < 1400; i++) {
    Stone n;
    n.pick_random();
    line.push_back(n);
  }
  while (elements--) {
    int col;
    while (1) {
      col = rand() % 7 + 1;
      if (at(0, col - 1) == ' ')
	break;
    }
    Stone element = line.front();
    line.pop_front();
    *this = drop(element, col);
  }
}
