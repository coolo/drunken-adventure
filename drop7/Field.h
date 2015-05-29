#include <string>

class Field {

public:
  Field();
  Field(const Field &f);
  static Field from_string(const char *str);
  std::string to_string() const;
  char at(int y, int x) const;
  void set(int y, int x, char c);
  
  // drops a '1'..'7' in col (note it's x+1)
  Field drop(char c, int col) const;

  // remove right ones
  bool blink();

  // next level
  bool add_B_row();
  void finalize_random();
  double rating() const;
  double recursive_rating(int depth) const;

  int score() const { return m_score; }
  int elements() const;
  void set_random(int elements = 11);
  
  void ann_input(float *arr) const;
  
private:
  bool gravitate();
  bool check_row(int y, int start_x, char c, char *marked);
  bool check_col(int start_y, int x, char c, char *marked);
  void markturn(int y, int x, char *marked);
  char data[50];
  int m_score;
  int m_step;
  int m_level;
  int m_dots;
};
