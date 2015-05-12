all: run_field_test

run_field_test: field_test
	./field_test

field_test: field_test.cpp Field.cpp
	g++ -Wall -Werror -o field_test field_test.cpp Field.cpp $(shell /usr/bin/gtest-config --ldflags --libs)

clean:
	rm *.o field_test
