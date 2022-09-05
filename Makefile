CC ?= gcc
LDFLAGS = -lcurses -ltinfo -static
STD = -ansi -std=c90 -pedantic

all:
	${CC} -g tc.c ${STD} ${LDFLAGS} -o tc
