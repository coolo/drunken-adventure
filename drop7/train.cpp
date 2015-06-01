#include "fann.h"
#include "Field.h"
#include <iostream>
#include <string.h>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <map>

using namespace std;

struct Step {
  float input[49*9];
  float output;
};
  
int main(int argc, char **argv)
{
  int hidden = 150;
  if (argc > 1)
    hidden = atoi(argv[1]);
  
  const unsigned int num_input = 49 * 9;
  const unsigned int num_output = 1;

  struct fann *ann;
  ann = fann_create_from_file("drop7.net");
  if (!ann) {
    //srand(time(0) + getpid());
    srand(0);
    ann = fann_create_standard(3, num_input, 160, num_output);
    fann_randomize_weights(ann, -0.1, 0.1);
  }
  
  fann_set_activation_function_hidden(ann, FANN_ELLIOT_SYMMETRIC);
  fann_set_activation_function_output(ann, FANN_GAUSSIAN);

  fann_set_training_algorithm(ann, FANN_TRAIN_QUICKPROP);

  int level = 3000;
  int epsilon = 10;
  vector<Step> steps;
  long total_score = 0;
  list<long> scores;
  while (level--) {
    Field f;
    srand(1);
    f.set_random();
    srand(time(0));
    //cerr << f.to_string();
    
    while (1) {
      Stone choice = f.next_stone();

      if (rand() % 100 < epsilon) {
	int rcol = rand() % 7;
	if (!f.stone(0, rcol).is_null())
	  continue;
	f = f.drop(choice, rcol + 1);
	while (f.blink());
	continue;
      }
      float best = -1;
      float factor = 0.9;
      int best_col = 0;
      
      for (int col = 1; col <= 7; ++col) {
	if (!f.stone(0, col - 1).is_null())
	  continue;
	
	Field n = f.drop(choice, col);
	float inputs[49*9];
	n.ann_input(inputs);
	
	float *calc = fann_run(ann, inputs);
	//cout << "calc " << int(calc[0] * 100) << endl;
		
	if (calc[0] > best || (calc[0] == best && rand() % 5 > 3)) {
	  best = calc[0];
	  best_col = col;
	}
	
      }
      
      if (best_col == 0) { // lost
	total_score += f.score();
	scores.push_back(f.score());
	if (scores.size() > 100) {
	  total_score -= scores.front();
	  scores.pop_front();
	}
	cout << "STEPS " << steps.size() << " " << f.score() << " " << total_score / scores.size() << endl;
	fann_train_data *trainingSet;
	trainingSet = fann_create_train(steps.size(), 49 * 9, 1);
	vector<Step>::const_iterator it = steps.begin();
	int counter = 0;
	float goal = 0;
	if (scores.size() == 1)
	  goal = 0.5;
	if (f.score() > long(total_score / scores.size()))
	  goal = 1;
	if (f.score() * 2 < long(total_score / scores.size())) {
	  cerr << "FAIL\n";
	  level = 100;
	}
	for (; it != steps.end(); ++it, ++counter) {
	  for (int i = 0; i < 49 * 9; ++i) {
	    trainingSet->input[counter][i] = it->input[i];
	  }
	  float error = goal - it->output;
	  //printf("OUTPUT %f-%f\n", it->output, error * pow(factor, steps.size() - counter - 1));
	  trainingSet->output[counter][0] = it->output + error * pow(factor, steps.size() - counter - 1);
	  //printf("OUTPUT-FAIL %f\n", trainingSet->output[counter][0] );
	}
	fann_train_epoch(ann, trainingSet);
	steps.clear();
	char buffer[30];
	sprintf(buffer, "drop7.net.%d", hidden);
	fann_save(ann, buffer);
	fann_destroy_train(trainingSet);
	//fann_print_connections(ann);
	break;
      }

      Step s;
      
      //cerr << "BEFORE " << endl << f.to_string();
      f = f.drop(choice, best_col);

      f.ann_input(s.input);
      float *calc = fann_run(ann, s.input);
      s.output = calc[0];
      steps.push_back(s);
      
      //cerr << "DROP " << choice.to_string() << " " << best_col << " " << s.output << endl << f.to_string();
      //f.finalize_random();
      //cerr << "RANDOM " << endl << f.to_string();
      while (f.blink()) {
	//cerr << "BLINKED " << endl << f.to_string();
	//f.finalize_random();
      }
      
      if (f.elements() == 0) { // emptied
	cout << "STEPS - WIN " << steps.size() << endl;
	fann_train_data *trainingSet;
	trainingSet = fann_create_train(steps.size(), 49 * 9, 1);
	vector<Step>::const_iterator it = steps.begin();
	int counter = 0;
	for (; it != steps.end(); ++it, counter++) {
	  for (int i = 0; i < 49 * 9; ++i) {
	    trainingSet->input[counter][i] = it->input[i];
	  }
	  float error = 1 -  it->output;
	  //printf("OUTPUT %f-%f\n", it->output, error * pow(factor, steps.size() - counter - 1));
	  trainingSet->output[counter][0] = it->output + error * pow(factor, steps.size() - counter - 1);
	  //printf("OUTPUT-WIN %f\n", trainingSet->output[counter][0]);
	}
	fann_train_epoch(ann, trainingSet);
	steps.clear();
      }

      //cerr << "BLINKEDF " << f.elements() << endl;
      //cerr << endl << endl;
    }
  }
  
  fann_destroy(ann);
  
  return 0;
}
