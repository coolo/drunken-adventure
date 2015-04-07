all: record

record: record.cpp
	g++ record.cpp -Wall -g -O2 $$(pkg-config --libs opencv)  -o record
