authmanager: main.o filebuffer.o cube2crypto.o
	g++ -Wall -g -o $@ $^ `pkg-config --cflags --libs --static gtk+-2.0` -export-dynamic
	
main.o: main.cpp filebuffer.h auth_model.cpp event_handlers.cpp
	g++ -c -Wall -g -o $@ $< `pkg-config --cflags --libs --static gtk+-2.0` -export-dynamic
	
filebuffer.o: filebuffer.cpp filebuffer.h
	g++ -c -Wall -g -o $@ $<
	
cube2crypto.o: cube2crypto.cpp shared/crypto.cpp shared/tools.h
	g++ -c -Wall -g -o $@ $<

w32distrib: authmanager authmanager.glade
	cp authmanager w32dist/
	cp -r Icons w32dist/
	cp authmanager.glade w32dist/

clean:
	rm -rf *.o authmanager