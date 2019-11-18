CC= /usr/bin/gcc

all:	udpclient udpserver

udpclient: udpclient.c stop_and_wait.o
	${CC} -g udpclient.c stop_and_wait.o -o udpclient

udpserver: udpserver.c
	${CC} -g udpserver.c stop_and_wait.o -o udpserver -lm
stop_and_wait.o: stop_and_wait.h stop_and_wait.c 
	${CC} -g -c stop_and_wait.c
clean:
	rm udpclient udpserver
