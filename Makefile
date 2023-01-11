CC = gcc
CFLAGS = -Os -std=c2x -Wall
LDFLAGS = -lcurses

all:
	ib dict.h.ib
	ib tc.c.ib -i --flags "${CFLAGS} ${LDFLAGS}"
clean:
	rm -rf build/*
