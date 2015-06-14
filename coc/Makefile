all: record takescreenshot

record: record.cpp
	g++ record.cpp -Wall -g -O2 $$(pkg-config --libs opencv)  -o record

takescreenshot: takescreenshot.cpp
	g++ takescreenshot.cpp -Wall -g -O2 $$(pkg-config --libs opencv)  -o takescreenshot
