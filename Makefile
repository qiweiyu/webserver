CC = gcc -g

default : server clean

server : util/qStack.o util/qLinkList.o util/qString.o util/qMalloc.o util/qDict.o util/qIo.o util/qEvent.o http.o

.PHONY : clean

clean :
	cd util && rm *.o
