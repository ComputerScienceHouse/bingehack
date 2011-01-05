CFLAGS=-Wall

all: build

build: board bd rx

rx: rx.o list.o
	$(CC) rx.o list.o -o rx
board: board.o list.o
	$(CC) board.o list.o -o board -lncurses

bd: bd.o list.o
	$(CC) bd.o list.o -o bd

clean:
	rm -fr board.o list.o board bd.o bd
