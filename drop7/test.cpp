#include "fann.h"
#include "Field.h"
#include <iostream>
#include <string.h>
#include <vector>

using namespace std;

struct Step {
  float input[49*9];
  float output;
};
  
int main(int argc, char **argv)
{
  const char *filename = "drop7.net";
  if (argc > 1)
    filename = argv[1];
  
  struct fann *ann;
  ann = fann_create_from_file(filename);
  if (!ann) 
    exit(1);
  
  const int max_level = 80;
  int level = max_level;
  long total_score = 0;

  while (level--) {
    Field f;
    srand(level);
    f.set_random();
    
    while (1) {
      char choice = rand() % 8;
      switch (choice) {
      case 0:
	choice = 'B';
	break;
      default:
	choice = '0' + choice;
      }
      
      float best = -1;
      int best_col = 0;
      
      for (int col = 1; col <= 7; ++col) {
	if (f.at(0, col - 1) != ' ')
	  continue;
	
	Field n = f.drop(choice, col);
	float inputs[49*9];
	n.ann_input(inputs);
	//cout << "NEW\n" << n.to_string();
	
	float *calc = fann_run(ann, inputs);
	//cout << "calc " << int(calc[0] * 100) << endl;
		
	n.finalize_random();
	while (n.blink())
	  n.finalize_random();
	
	if (calc[0] > best || (calc[0] == best && rand() % 5 > 3)) {
	  best = calc[0];
	  best_col = col;
	}
	
      }
      
      if (best_col == 0) { // lost
	cout << "STEPS " << f.score() << endl;
	total_score += f.score();
	break;
      }

      //cerr << "BEFORE " << endl << f.to_string();
      f = f.drop(choice, best_col);

      //cerr << "DROP " << choice << " " << best_col << " " << s.output << endl << f.to_string();
      f.finalize_random();
      //cerr << "RANDOM " << endl << f.to_string();
      while (f.blink()) {
	//cerr << "BLINKED " << endl << f.to_string();
	f.finalize_random();
      }
      
    }
  }
  
  fann_destroy(ann);
  cout << "TOTAL: " << total_score / max_level << endl;
  
  return 0;
}
