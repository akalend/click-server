LIBS = -levent  -L/usr/local/lib
INCLUDES = -I/usr/local/include -I/opt/local/include    
MONGOINC = -I/Users/akalend/dist/mongodb-o++  
MONGOLIB = -L/opt/local/lib -lmongoclient -lboost_thread-mt -lboost_filesystem  -lboost_system-mt -lboost_program_options-mt -ljs

main: click-server.o
	g++ -o click-server click-server.o  $(LIBS) $(MONGOLIB)

click-server.o:
	g++ -o click-server.o  -c main.cpp $(INCLUDES) $(MONGOINC)

install: click-server 
	sudo cp click-server /usr/local/bin/
	sudo cp clicker.conf /usr/local/etc/

debug:
	g++ -ggdb -o click-server main.cpp -levent  -L/usr/local/lib -I/usr/local/include 

re2c:
	re2c -o conf.c conf.re.c 

run:
	./click-server clicker.conf
clean:	
	rm *.o
	rm click-server

mongo:
	g++ -o outputMongoStorage.o outputMongoStorage.cpp  $(INCLUDES)

