CC = gcc
CFLAGS = -Os -std=c2x -Wall -g -fanalyzer
LDFLAGS = -lcurses

all:
	ib tc.c.ib -i --flags "${CFLAGS} ${LDFLAGS}"
clean:
	rm -rf *.c *.h tc
