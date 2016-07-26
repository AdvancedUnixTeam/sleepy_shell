# Makefile template for shared library

CC = gcc # C compiler
CFLAGS = -fPIC -Wall -Wextra -O2 -g # C flags
LDFLAGS = -shared  # linking flags
RM = rm -f  # rm command
TARGET_LIB = libtarget.so # target lib

SRCS = main.c shell.c # source files
OBJS = $(SRCS:.c=.o)

.PHONY: all

all: shell
 
shell: ${TARGET_LIB} main.o shell.o
	$(CC) main.o shell.o -o shell

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	-${RM} ${TARGET_LIB} ${OBJS} ${SRCS:.c=.d} shell
