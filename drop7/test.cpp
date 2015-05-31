#include "fann.h"
#include "Field.h"
#include <iostream>
#include <string.h>
#include <vector>
#include <map>
#include <unordered_set>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

int main(int argc, char **argv)
{
  const char *filename = "drop7.net";
  if (argc > 1)
    filename = argv[1];
  
  struct fann *ann;
  ann = fann_create_from_file(filename);
  if (!ann) 
    exit(1);

  srand(time(0) + getpid());
  const int max_level = 1000;
  int level = max_level;
  long total_score = 0;
  long max_score = 0;
  
  map<string, long> hashes;
  
  while (level--) {
    Field f;
    f.set_random();

    unordered_set<string> states;
    
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
		
	if (calc[0] > best || (calc[0] == best && rand() % 5 > 3)) {
	  best = calc[0];
	  best_col = col;
	}
	
      }
      
      if (best_col == 0) { // lost
	cout << "STEPS " << f.score() << endl;
	total_score += f.score();
	if (f.score() > max_score)
	  max_score = f.score();
	break;
      }

      f = f.drop(choice, best_col);
      states.insert(f.to_string());
      f.finalize_random();

      while (f.blink()) {
	f.finalize_random();
      }
      
    }
    int myscore = f.score();
    unordered_set<string>::const_iterator it = states.begin();
    for (; it != states.end(); ++it) {
      if (myscore > hashes[*it])
	hashes[*it] = myscore;
    }
  }
  
  cout << "TOTAL: " << total_score / max_level << " " << hashes.size() << endl;

  fann_train_data *train_data;
  train_data = fann_create_train(hashes.size(), 49 * 9, 1);
  map<string,long>::const_iterator it = hashes.begin();
  int counter = 0;
  for (; it != hashes.end(); ++it, ++counter) {
    Field f = Field::from_string(it->first.c_str());
    f.ann_input(train_data->input[counter]);
    float goal = sqrt(it->second / 500000.);
    if (goal > 1)
      goal = 1;
    train_data->output[counter][0] = goal;
  }
  fann_set_train_stop_function(ann, FANN_STOPFUNC_BIT);
  fann_train_on_data(ann, train_data, 300, 10, 7);
  fann_save(ann, "retrained.net");
  return 0;
}
