CC ?= gcc
LDFLAGS = -lcurses -ltinfo
STD = -std=c99 -pedantic -Wall

all:
	${CC} -g tc.c ${LDFLAGS} ${STD} -o tc
