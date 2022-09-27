#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#define FONT_DATA_SIZE (0x10 * 0x5)

unsigned short pc = 0x200;
unsigned char ram[0x1000] = { 0 };
unsigned char v_reg[0x10] = { 0 };
unsigned short i_reg = 0x1234;
unsigned char call_stack[0x100] = { 0 };
unsigned char delay_timer = 0;
unsigned char sound_timer = 0;
unsigned char display[0x40 * 0x20] = { 0 };

unsigned char font_data[FONT_DATA_SIZE] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,
        0x20, 0x60, 0x20, 0x20, 0x70,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x10, 0xF0,
        0x90, 0x90, 0xF0, 0x10, 0x10,
        0xF0, 0x80, 0xF0, 0x10, 0xF0,
        0xF0, 0x80, 0xF0, 0x90, 0xF0,
        0xF0, 0x10, 0x20, 0x40, 0x40,
        0xF0, 0x90, 0xF0, 0x90, 0xF0,
        0xF0, 0x90, 0xF0, 0x10, 0xF0,
        0xF0, 0x90, 0xF0, 0x90, 0x90,
        0xE0, 0x90, 0xE0, 0x90, 0xE0,
        0xF0, 0x80, 0x80, 0x80, 0xF0,
        0xE0, 0x90, 0x90, 0x90, 0xE0,
        0xF0, 0x80, 0xF0, 0x80, 0xF0,
        0xF0, 0x80, 0xF0, 0x80, 0x80
};

void
usage(void) {
        puts("Usage: c8 [FILE]");
}

int
main(int argc, char* argv[]) {
        char c;
        int i;
        FILE* fp;

        /* Set up SDL. */
        SDL_Init(SDL_INIT_VIDEO);

        /* Load font data. */
        for (i = 0; i < FONT_DATA_SIZE; i++) {
                ram[i] = font_data[i];
        }

        /* Load rom. */
        if (optind >= argc) {
                usage();
                exit(EXIT_FAILURE);
        }

        fp = fopen(argv[optind], "r");
        c = fgetc(fp);
        i = 0;
        while (c != EOF) {
                if (0x200 + i >= 0x1000) {
                        puts("Error: ROM too large.");
                        exit(EXIT_FAILURE);
                }
                ram[0x200 + i] = c;
                c = fgetc(fp);
                i++;
        }

        /* Run program. */
        for (i = 0x200; i < 0x1000; i++) {
                printf("%x ", ram[i]);
        }
        
        putchar('\n');

        exit(EXIT_SUCCESS);
}