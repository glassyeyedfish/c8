c8:
	cc -ansi -pedantic -Wall -Wextra -Werror \
	src/*.c \
	-o build/c8 \
	-lSDL2 \