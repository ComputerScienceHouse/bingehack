CFLAGS=-Wall

all: build

build: board

board: board.o list.o
	$(CC) board.o list.o -o board -lncurses

clean:
	rm -fr board.o list.o board
