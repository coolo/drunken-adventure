all: a.out run_field_test

run_field_test: field_test
	./field_test

a.out: drop7.cpp Field.cpp Field.h
	g++ -g3 -Wall -Werror Field.cpp drop7.cpp

field_test: field_test.cpp Field.cpp Field.h
	g++ -Wall -Werror -o field_test field_test.cpp Field.cpp $(shell /usr/bin/gtest-config --ldflags --libs)

clean:
	rm *.o field_test
