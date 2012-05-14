authmanager: main.cpp filebuffer.o cube2crypto.o
	g++ -Wall -g -o $@ $^ `pkg-config --cflags --libs --static gtk+-2.0` -export-dynamic
	
filebuffer.o: filebuffer.cpp filebuffer.h
	g++ -c -Wall -g -o $@ $<
	
cube2crypto.o: cube2crypto.cpp shared/crypto.cpp shared/tools.h
	g++ -c -Wall -g -o $@ $<
