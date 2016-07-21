CC = gcc
HEADERS = shell.h

all: shell

shell: main.o
	$(CC) main.o  -o shell

main.o: main.c
	$(CC) -c main.c

clean: 
	rm -f *.o shell
