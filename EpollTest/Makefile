TARGETS = client server
CC = g++
CCFLAGS = -g -I${USER_INCLUDE} -L${USER_LIB} -I/home/kwang/Documents/Matrix/EpollTest

LIBFLAGS = -lpthread

all:	$(TARGETS)

%.o: %.cpp
	$(CC) $(CCFLAGS) -c $^ $(LIBFLAGS)



net.o: net.cpp
matrix_epoll_server.o: matrix_epoll_server.cpp
client.o: client.cpp
serverside.o: serverside.cpp

client: client.o net.o matrix_epoll_server.o
	$(CC) $(CCFLAGS) -o $@ $^ $(LIBFLAGS)


server: serverside.o net.o matrix_epoll_server.o
	$(CC) $(CCFLAGS) -o $@ $^ $(LIBFLAGS)




.PHONY:	clean

clean:	
	rm *.o
	rm $(TARGETS)