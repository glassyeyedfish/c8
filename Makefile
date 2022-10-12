CC = cc
CFLAGS = -ansi -pedantic -Wall -Wextra -g
LDFLAGS = -lSDL2

SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)

all: build/ c8

build/:
	mkdir -p build

build/%.o: src/%.c
		${CC} -c ${CFLAGS} $< -o $@

c8: ${OBJ}
		${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
		rm build/*
		rm c8
