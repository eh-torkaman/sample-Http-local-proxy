CC=g++
CFLAGS= -g -Wall -Werror   -pthread
all:proxy
proxy: proxy.cpp
	$(CC) $(CFLAGS)   -o ./bin/proxy_parse.o -c proxy_parse.c 
	$(CC)  -o ./bin/md5.o -c  md5.cpp
	$(CC)  -o ./bin/stuff.o -c  stuff.cpp
	$(CC)   $(CFLAGS)   -o ./bin/proxy.o -c proxy.cpp  
	$(CC) $(CFLAGS)   -o proxy ./bin/md5.o  ./bin/stuff.o ./bin/proxy_parse.o ./bin/proxy.o  
	

clean:
	rm -f proxy *.o




