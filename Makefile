CC = clang
LDFLAGS = -lcurses -ltinfo
STD = -std=c11 -pedantic -Weverything

all:
	${CC} -g tc.c ${LDFLAGS} ${STD} -o tc
