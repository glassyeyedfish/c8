c8:
	cc -ansi -pedantic -Wall -Wextra -Werror -g \
	src/*.c \
	-o build/c8 \
	-lSDL2 \