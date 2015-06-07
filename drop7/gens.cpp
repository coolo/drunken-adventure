#include "fann.h"
#include "Field.h"
#include <iostream>
#include <string.h>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <map>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

struct Step {
  float input[49*9];
  float output;
};

void random_seed() {
  int fd = open("/dev/urandom", O_RDONLY);
  unsigned int buffer[1];
  read(fd, buffer, sizeof(unsigned int));
  close(fd);
  srand(buffer[0]);
}

long testAnn(struct fann *ann, const Field &_f)
{
  Field f = _f;
  
  while (1) {
    Stone choice = f.next_stone();
    
    float best = -1;
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
    
    if (best_col == 0) // lost
      return f.score();
    
    //cerr << "BEFORE " << endl << f.to_string();
    f = f.drop(choice, best_col);
    
    //cerr << "RANDOM " << endl << f.to_string();
    while (f.blink());
  }
  
}

void trainAnn(struct fann *ann)
{
  int level = 10;
  vector<Step> steps;
  long total_score = 0;
  list<long> scores;
  while (level--) {
    Field f;
    srand(level);
    f.set_random();
    random_seed();
    //cerr << f.to_string();
    
    while (1) {
      Stone choice = f.next_stone();

      float best = -1;
      float factor = 0.95;
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
      
      //cerr << "BLINKEDF " << f.elements() << endl;
      //cerr << endl << endl;
    }
  }

}


int main(int argc, char **argv)
{
  const unsigned int num_input = 49 * 9;
  const unsigned int num_output = 1;

  const int individuals = 100;
  struct fann *generation[individuals];
  random_seed();

  for (int i = 0; i < individuals; i++) {
    generation[i] = fann_create_standard(3, num_input, 160, num_output);
    fann_randomize_weights(generation[i], -1, 1);
  
    fann_set_activation_function_hidden(generation[i], FANN_ELLIOT_SYMMETRIC);
    fann_set_activation_function_output(generation[i], FANN_GAUSSIAN);
    
    fann_set_training_algorithm(generation[i], FANN_TRAIN_QUICKPROP);

    trainAnn(generation[i]);
  }

  while (true) {
    Field f;
    f.set_random();
  
  long scores[individuals];
  bool deads[individuals];
  for (int i = 0; i < individuals; i++) {
    scores[i] = 0;
    deads[i] = false;
  }
  int kills = 0;
  
  while (kills < individuals / 3) {
    int i1 = rand() % individuals;
    while (deads[i1])
      i1 = rand() % individuals;
    int i2 = i1;
    while (i1 == i2 || deads[i2]) {
      i2 = rand() % individuals;
    }
    scores[i1] |= testAnn(generation[i1], f);
    scores[i2] |= testAnn(generation[i2], f);

    int kill = i1;
    if (scores[i1] > scores[i2])
      kill = i2;
    
    cerr << i1 << " " << scores[i1] << " " << i2 << " " << scores[i2] << " killing " << kill << endl;
    deads[kill] = true;
    kills++;
  }

  for (int i = 0; i < individuals; i++) {
    scores[i] = 0;
    if (deads[i]) {
      int i1 = rand() % individuals;
      while (deads[i1])
	i1 = rand() % individuals;
      int i2 = i1;
      while (i1 == i2 || deads[i2]) {
	i2 = rand() % individuals;
      }
      cerr << "create child of " << i1 << " " << i2 << " as " << i << endl;
      deads[i] = false;
      struct fann_connection *c = (struct fann_connection *)malloc(sizeof(struct fann_connection) * fann_get_total_connections(generation[i]));
      struct fann_connection *c1 = (struct fann_connection *)malloc(sizeof(struct fann_connection) * fann_get_total_connections(generation[i1]));
      struct fann_connection *c2 = (struct fann_connection *)malloc(sizeof(struct fann_connection) * fann_get_total_connections(generation[i2]));
      fann_get_connection_array(generation[i1], c1);
      fann_get_connection_array(generation[i2], c2);
      unsigned int j = fann_get_total_connections(generation[i]);
      while (j > 0) {
	j--;
	if (rand() % 100 > 50) {
	  c[j].weight = c1[j].weight;
	} else {
	  c[j].weight = c2[j].weight;
	}
      }
      free(c1);
      free(c2);
      fann_set_weight_array(generation[i], c, fann_get_total_connections(generation[i]));
      trainAnn(generation[i]);
      free(c);
    }
  }
  }
  
  return 0;
}
