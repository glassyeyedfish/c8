CC = gcc
CFLAGS = -ansi -pedantic -Wall -Wextra -Werror -g
LDFLAGS = -lSDL2

SRC = c8.c sdl.c
OBJ = ${SRC:.c=.o}

.c.o:
		${CC} -c ${CFLAGS} $<

c8: ${OBJ}
		${CC} -o $@ ${OBJ} ${LDFLAGS}