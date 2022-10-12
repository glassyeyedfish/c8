#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "sdl.h"

#define VERSION_STIRNG "0.1.0"

#define FONT_DATA_SIZE (16 * 5)

sdl_context_t ctx = { 0 };

int flag_stepping = 0;

unsigned short pc = 0x200;
unsigned char ram[0x1000] = { 0 };
unsigned char v_reg[16] = { 0 };
unsigned short i_reg = 0;
unsigned char call_stack[0x100] = { 0 };
unsigned char delay_timer = 0;
unsigned char sound_timer = 0;
unsigned long display[32] = { 0 };

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

/* ROM functions */
void init_rom(void);
void run_rom(void);
void load_rom(char* filename);

/* Instruction */
void eval_instruction(void);

/* Debug printing functions */
void print_instruction(unsigned int pc);
void print_v_registers(void);

/* CLI arg print functions */
void print_usage(void);
void print_version(void);

void init_rom() {
        int i;

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

        for (i = 0; i < FONT_DATA_SIZE; i++) {
                ram[i] = font_data[i];
        }
}

void run_rom(void) {

        /* Initialize everything. */
        init_rom();


        /* Main loop. */
        while (!ctx.window_should_close) {
                /* Event handling. */
                poll_events(&ctx);

                eval_instruction();

                if (pc >= 0x1000) {
                        puts("Error: program counter above allocated memory.");
                        exit(EXIT_FAILURE);
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
eval_instruction(void) {
        /* Implements the step-through functionality. */
        if (
                (flag_stepping && is_key_pressed(&ctx, SDLK_SPACE)) 
                || !flag_stepping
        ) {

                /* Seperate instruction into important pieces. */
                unsigned int inst = (ram[pc] << 8) | ram[pc + 1];
                unsigned int addr = inst & 0x0FFF;
                unsigned int byte = inst & 0x00FF;
                unsigned int w = (inst & 0xF000) >> 12;
                unsigned int x = (inst & 0x0F00) >> 8;
                unsigned int y = (inst & 0x00F0) >> 4;
                unsigned int z = inst & 0x000F;

                /* Switch over specific nibbles to determine which 
                        * instruction should be executed, then exec it. */
                switch(w) {

                case 0x0:
                        break;

                case 0x1:
                        break;
                        
                case 0x2:
                        break;

                case 0x3:
                        break;

                case 0x4:
                        break;

                case 0x5:
                        break;

                /* Vx = byte */
                case 0x6:
                        v_reg[x] = byte;
                        break;

                /* Vx = Vx + kk */
                case 0x7:
                        v_reg[x] += byte;
                        break;

                case 0x8:
                        switch(z) {

                        /* Vx = Vy */
                        case 0x0:
                                v_reg[x] = v_reg[y];
                                break;

                        /* Vx = Vx | Vy */
                        case 0x1:
                                v_reg[x] = v_reg[x] | v_reg[y];
                                break;

                        /* Vx = Vx & Vy */
                        case 0x2:
                                v_reg[x] = v_reg[x] & v_reg[y];
                                break;

                        /* Vx = Vx ^ Vy */
                        case 0x3:
                                v_reg[x] = v_reg[x] ^ v_reg[y];
                                break;

                        case 0x4:
                                unsigned char temp = v_reg[x];
                                v_reg[x] = v_reg[x] + v_reg[y];
                                v_reg[0xF] = 
                                        v_reg[x] < temp || v_reg[x] < v_reg[y]
                                        ? 1
                                        : 0;
                                break;

                        case 0x5:
                                break;

                        case 0x6:
                                break;

                        case 0x7:
                                break;

                        case 0xE:
                                break;

                        default:
                                break;
                        }
                        break;

                case 0x9:
                        break;

                case 0xA:
                        break;

                case 0xB:
                        break;

                case 0xC:
                        break;

                case 0xD:
                        break;

                case 0xE:
                        break;

                case 0xF:
                        break;

                default:
                        break;
                }

                print_instruction(pc);
                print_v_registers();
                pc += 2;
        }
}

/* Takes a file path and trie to load the bytes into the virtual ram. Prints 
 * an error and exits if the file doesn't exsist of if the file it too big to 
 * be loaded. */
void load_rom(char* filename) {
        char c;
        long i;

        FILE* fp;
        long size;

        fp = fopen(filename, "r");
        if(fp == NULL) {
                printf("Error: Could not load '%s'\n", filename);
                exit(EXIT_FAILURE);
        }

        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        rewind(fp);

        c = fgetc(fp);
        i = 0;
        while (i < size) {
                if (0x200 + i >= 0x1000) {
                        puts("Error: ROM too large");
                        exit(EXIT_FAILURE);
                }
                ram[0x200 + i] = c;
                c = fgetc(fp);
                i++;
        }
}

void print_instruction(unsigned int pc) {
        unsigned int i;
        for (i = pc; i <= pc + 1; i++) {
                if (ram[i] < 0x10) {
                        putchar('0');
                }
                printf("%x", ram[i]);
        }
        putchar('\n');
}

void print_v_registers(void) {
        int i;
        for (i = 0; i < 16; i++) {
                printf("V%x = %x ", i, v_reg[i]);
        }
        putchar('\n');
}

void print_usage(void) {
        puts("Usage: c8 [OPTIONS...] [FILE]");
        putchar('\n');
        puts("    -s    step through one instruction at a time");
        puts("    -h    print this list and exit");
        puts("    -V    print the version number and exit");
}


void print_version(void) {
        printf("c8: v%s\n", VERSION_STIRNG);
}

int main(int argc, char* argv[]) {
        int i;

        /* Process CLI arguments. */
        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-s") == 0) {
                        flag_stepping = 1;
                } else if (strcmp(argv[i], "-h") == 0) {
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
