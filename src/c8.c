#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "sdl.h"

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
        context_t ctx = { 0 };

        /* Initialize everything. */
        SDL_Init(SDL_INIT_VIDEO);

        ctx.window = SDL_CreateWindow(
                "Chip-8",
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                640,
                320,
                0
        );

        ctx.renderer = SDL_CreateRenderer(ctx.window, -1, 0);
        SDL_RenderSetScale(ctx.renderer, 10.0, 10.0);

        ctx.window_should_close = 0;


        /* Main loop. */
        while (!ctx.window_should_close) {

                /* Event handling. */
                poll_events(&ctx);

                if (is_key_pressed(&ctx, SDLK_SPACE)) {
                        SDL_SetRenderDrawColor(ctx.renderer, 255, 0, 0, 255);
                } else {
                        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 255);
                }
                SDL_RenderClear(ctx.renderer);

                SDL_RenderPresent(ctx.renderer);

                SDL_Delay(16);
        }


        /* Clean up */
        SDL_DestroyRenderer(ctx.renderer);
        SDL_DestroyWindow(ctx.window);

        SDL_Quit();
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
        putchar('\n');
        puts("    -h    print this list and exit");
        puts("    -V    print the version number and exit");
}


void
print_version(void) {
        printf("c8: v%s\n", VERSION_STIRNG);
}

int
main(int argc, char* argv[]) {
        int i;

        /* Process CLI arguments. */
        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-h") == 0) {
                        print_usage();
                        exit(EXIT_SUCCESS);
                } else if (strcmp(argv[i], "-V") == 0) {
                        print_version();
                        exit(EXIT_SUCCESS);
                } else {
                        load_rom(argv[i]);
                        run_rom();
                        exit(EXIT_SUCCESS);
                }
        }

        puts("Error: failed to parse arguments, use '-h' for usage");
        exit(EXIT_FAILURE);
}
