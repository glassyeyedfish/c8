#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#define VERSION_STIRNG "0.1.0"

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
run_rom(void) {
        int i;
        for (i = 0x200; i < 0x208; i++) {
                printf("%x\n", ram[i]);
                SDL_Delay(16);
        }
        putchar('\n');
}

void
load_rom(char* filename) {
        char c;
        int i;
        FILE* fp;

        fp = fopen(filename, "r");
        if(fp == NULL) {
                printf("Error: Could not load '%s'\n", filename);
                exit(EXIT_FAILURE);
        }
        c = fgetc(fp);
        i = 0;
        while (c != EOF) {
                if (0x200 + i >= 0x1000) {
                        puts("Error: ROM too large");
                        exit(EXIT_FAILURE);
                }
                ram[0x200 + i] = c;
                c = fgetc(fp);
                i++;
        }
}

void
print_usage(void) {
        puts("Usage: c8 [OPTIONS...] [FILE]");
}


void
print_version(void) {
        printf("c8: v%s\n", VERSION_STIRNG);
}

void 
print_help(void) {
        print_usage();
        putchar('\n');
        puts("    -h    print this list and exit");
        puts("    -V    print the version number and exit");
}

int
main(int argc, char* argv[]) {
        int i;
        char loaded_rom;

        /* Set up SDL. */
        SDL_Init(SDL_INIT_VIDEO);

        /* Load font data. */
        for (i = 0; i < FONT_DATA_SIZE; i++) {
                ram[i] = font_data[i];
        }

        /* Process CLI arguments. */
        loaded_rom = 0;
        for (i = 1; i < argc && !loaded_rom; i++) {
                if (strcmp(argv[i], "-h") == 0) {
                        print_help();
                        exit(EXIT_SUCCESS);
                } else if (strcmp(argv[i], "-V") == 0) {
                        print_version();
                        exit(EXIT_SUCCESS);
                } else {
                        load_rom(argv[i]);
                        loaded_rom = 1;
                }
        }

        if (!loaded_rom) {
                print_usage();
                exit(EXIT_FAILURE);
        }

        run_rom();
        
        exit(EXIT_SUCCESS);
}