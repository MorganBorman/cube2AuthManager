ifeq ($(OS),Windows_NT)
	#Windows stuff
	FLAGS = `pkg-config --cflags --libs --static gtk+-2.0` -mwindows
	RESOURCE_FILE = authmanager.res
else
	#Linux stuff
	FLAGS = `pkg-config --cflags --libs --static gtk+-2.0` -export-dynamic
	RESOURCE_FILE = 
endif

authmanager: main.o ${RESOURCE_FILE} filebuffer.o cube2crypto.o
	g++ -Wall -g -o $@ $^ ${FLAGS}
	
main.o: main.cpp filebuffer.h auth_model.cpp event_handlers.cpp
	g++ -c -Wall -g -o $@ $< ${FLAGS}
	
filebuffer.o: filebuffer.cpp filebuffer.h
	g++ -c -Wall -g -o $@ $<
	
cube2crypto.o: cube2crypto.cpp shared/crypto.cpp shared/tools.h
	g++ -c -g -o $@ $<

w32distrib: authmanager authmanager.glade
	cp authmanager w32dist/
	cp authmanager.glade w32dist/
	cmd //c "start w32dist\\package.bat"
	
${RESOURCE_FILE}: authmanager.rc w32dist/Icons/Gnome-dialog-password.ico
	windres $< -O coff -o $@

clean:
	rm -rf *.o authmanager ${RESOURCE_FILE}