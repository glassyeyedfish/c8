CC = cc
CFLAGS = -ansi -pedantic -Wall -Wextra -Werror -g
LDFLAGS = -lSDL2

SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=build/%.o)

all: c8

c8: ${OBJ}
		${CC} -o $@ ${OBJ} ${LDFLAGS}

build/%.o: src/%.c
		mkdir -p build
		${CC} -c ${CFLAGS} $< -o $@

clean:
		rm build/*
		rm c8
