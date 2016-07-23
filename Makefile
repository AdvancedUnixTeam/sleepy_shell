CC = gcc
HEADERS = shell.h

all: shell

shell: main.o shell.o
	$(CC) main.o shell.o  -o shell

shell.o: shell.c
	$(CC) -c shell.c

main.o: main.c
	$(CC) -c main.c

clean: 
	rm -f *.o shell
