CC = gcc
INC = include
LDFLAGS = -lcurses -ltinfo
STD = -std=c2x -pedantic -Wall

all:
	${CC} -I${INC} -c src/mem.c -o build/mem.o
	${CC} -I${INC} -c src/kb.c -o build/kb.o
	${CC} -I${INC} -c src/tc.c -o build/tc.o
	${CC} ${LDFLAGS} build/mem.o build/kb.o build/tc.o -o tc
