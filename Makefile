CC = gcc

CCFLAGS = -Wall -pedantic -g

LD = gcc

LDFLAGS =

all: mush

mush: mush.o parseline.o stage.o util.o

mush.o: mush.c
	$(CC) $(CCFLAGS) -c -o mush.o mush.c
parseline.o: parseline.c
	$(CC) $(CCFLAGS) -c -o parseline.o parseline.c
stage.o: stage.c
	$(CC) $(CCFLAGS) -c -o stage.o stage.c
util.o: util.c
	$(CC) $(CCFLAGS) -c -o util.o util.c
clean:
	rm *.o
