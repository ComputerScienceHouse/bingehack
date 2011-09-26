CFLAGS=-Wall -ggdb3

all: build

build: board rx

rx: rx.o list.o
	$(CC) rx.o list.o -o rx
board: board.o list.o
	$(CC) board.o list.o -o board -lncurses

clean:
	rm -fr board.o list.o board bd.o bd rx rx.o
