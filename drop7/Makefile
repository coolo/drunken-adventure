all: a.out train

run_field_test: field_test
	./field_test

a.out: drop7.cpp Field.cpp Field.h
	g++ -O3 -Wall -Werror Field.cpp drop7.cpp $$(pkg-config --libs opencv fann)

field_test: field_test.cpp Field.cpp Field.h
	g++ -Wall -Werror -o field_test field_test.cpp Field.cpp $(shell /usr/bin/gtest-config --ldflags --libs) 

train: train.cpp  Field.cpp Field.h
	g++ -O3 -g -Wall -Werror -o train train.cpp Field.cpp $$(pkg-config --libs fann)

clean:
	rm field_test a.out
