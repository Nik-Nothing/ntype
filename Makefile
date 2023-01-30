CC = gcc
CFLAGS = -Os -std=c2x -Wall -g -fanalyzer
LDFLAGS = -lcurses -ltinfo

all:
	ib tc.c.ib -in --flags "${CFLAGS} ${LDFLAGS}"
clean:
	rm -rf *.c *.h tc
