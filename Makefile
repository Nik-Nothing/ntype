CC ?= gcc
LDFLAGS = -lcurses
STD = -ansi -std=c90 -pedantic

all:
	${CC} -g tc.c ${STD} ${LDFLAGS} -o tc
