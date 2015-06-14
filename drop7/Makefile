all: a.out train gens

run_field_test: field_test
	./field_test

a.out: drop7.cpp Field.cpp Field.h
	g++ -O3 -Wall -Werror Field.cpp drop7.cpp $$(pkg-config --libs opencv fann)

field_test: field_test.cpp Field.cpp Field.h
	g++ -Wall -Werror -o field_test field_test.cpp Field.cpp $(shell /usr/bin/gtest-config --ldflags --libs) 

train: train.cpp  Field.cpp Field.h
	g++ -O3 -g -Wall -Werror -o train train.cpp Field.cpp $$(pkg-config --libs fann)

gens: gens.cpp Field.cpp Field.h
	g++ -O3 -g -Wall -Werror -o gens gens.cpp Field.cpp $$(pkg-config --libs fann)

test: test.cpp Field.cpp Field.h
	g++ -O3 -g -Wall -Werror -o test test.cpp Field.cpp -std=c++11 $$(pkg-config --libs fann)

athousand: athousand.cpp Field.cpp Field.h
	g++ -O3 -g -Wall -Werror -o athousand athousand.cpp Field.cpp -std=c++11

clean:
	rm field_test a.out
