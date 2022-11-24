#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "sdl.h"

#define C8_VERSION_STIRNG "0.1.0"

#define C8_FONT_DATA_SIZE (16 * 5)
#define C8_DISPLAY_SIZE (64 * 32)

#define C8_MAX_CALL_STACK (0x100)

sdl_context_t ctx = { 0 };

int flag_stepping = 0;

unsigned short pc = 0x200;
unsigned char ram[0x1000] = { 0 };

unsigned char v_reg[16] = { 0 };
unsigned short i_reg = 0;

unsigned short sp = 0;
unsigned char call_stack[C8_MAX_CALL_STACK] = { 0 };

unsigned char delay_timer = 0;
unsigned char sound_timer = 0;

unsigned char display[C8_DISPLAY_SIZE] = { 0 };

unsigned short keyboard = 0;

unsigned char font_data[C8_FONT_DATA_SIZE] = {
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
void print_display(void);
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

    for (i = 0; i < C8_FONT_DATA_SIZE; i++) {
        ram[i] = font_data[i];
    }
}

void run_rom(void) {
    int i;

    /* Initialize everything. */
    init_rom();

    /* Main loop. */
    while (!ctx.window_should_close) {
        /* Event handling. */
        poll_events(&ctx);

        /* Update keyboard data structure. */
        keyboard = 0;

        /* +---+---+---+---+ *
         * | 1 | 2 | 3 | C | *
         * +---+---+---+---+ *
         * | 4 | 5 | 6 | D | *
         * +---+---+---+---+ *
         * | 7 | 8 | 9 | E | *
         * +---+---+---+---+ *
         * | A | 0 | B | F | *
         * +---+---+---+---+ */

        /* +---+---+---+---+ *
         * | 1 | 2 | 3 | 4 | *
         * +---+---+---+---+ *
         * | Q | W | E | R | *
         * +---+---+---+---+ *
         * | A | S | D | F | *
         * +---+---+---+---+ *
         * | Z | X | C | V | *
         * +---+---+---+---+ */

        if (is_key_down(&ctx, KEY_1)) keyboard |= 1 << 0x1;
        if (is_key_down(&ctx, KEY_2)) keyboard |= 1 << 0x2;
        if (is_key_down(&ctx, KEY_3)) keyboard |= 1 << 0x3;
        if (is_key_down(&ctx, KEY_4)) keyboard |= 1 << 0xC;
        if (is_key_down(&ctx, KEY_Q)) keyboard |= 1 << 0x4;
        if (is_key_down(&ctx, KEY_W)) keyboard |= 1 << 0x5;
        if (is_key_down(&ctx, KEY_E)) keyboard |= 1 << 0x6;
        if (is_key_down(&ctx, KEY_R)) keyboard |= 1 << 0xD;
        if (is_key_down(&ctx, KEY_A)) keyboard |= 1 << 0x7;
        if (is_key_down(&ctx, KEY_S)) keyboard |= 1 << 0x8;
        if (is_key_down(&ctx, KEY_D)) keyboard |= 1 << 0x9;
        if (is_key_down(&ctx, KEY_F)) keyboard |= 1 << 0xE;
        if (is_key_down(&ctx, KEY_Z)) keyboard |= 1 << 0xA;
        if (is_key_down(&ctx, KEY_X)) keyboard |= 1 << 0x0;
        if (is_key_down(&ctx, KEY_C)) keyboard |= 1 << 0xB;
        if (is_key_down(&ctx, KEY_V)) keyboard |= 1 << 0xF;

        /* Implements the step-through functionality. */
        if (
            (flag_stepping && is_key_pressed(&ctx, KEY_SPACE)) 
            || !flag_stepping
        ) {
            eval_instruction();
        }

        if (pc >= 0x1000) {
            puts("Error: program counter above allocated memory.");
            exit(EXIT_FAILURE);
        }

        /* Update diplay. */
        SDL_RenderClear(ctx.renderer);
        for (i = 0; i < C8_DISPLAY_SIZE; i++) {
            unsigned int color = display[i] != 0 ? 255 : 0;
            SDL_SetRenderDrawColor(ctx.renderer, color, color, color, 255);
            SDL_RenderDrawPoint(ctx.renderer, i % 64, (int) i / 64);
        }
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
    unsigned int row, col;

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
        /* Clear screen */
        if (addr == 0x0E0) {
            int i;
            for (i = 0; i < C8_DISPLAY_SIZE; i++) {
                display[i] = 0;
            }
        /* return from subroutine */
        } else if (addr == 0x0EE) {
            if (sp <= 0) {
                puts("Error: stack underflow");
                exit(EXIT_FAILURE);
            }
            pc = call_stack[sp];
            sp--;
        }
        break;

    /* jump to addr */
    case 0x1:
        pc = addr - 2;
        break;
        
    /* call subroutine at addr */
    case 0x2:
        if (sp >= C8_MAX_CALL_STACK) {
            puts("Error: stack overflow");
            exit(EXIT_FAILURE);
        }
        call_stack[sp] = pc;
        sp++;
        pc = addr - 2;
        break;

    /* skip if Vx = byte */
    case 0x3:
        if (v_reg[x] == byte) pc += 2;
        break;

    /* skip if Vx != byte */
    case 0x4:
        if (v_reg[x] != byte) pc += 2;
        break;

    /* skip if Vx == Vy */
    case 0x5:
        if (v_reg[x] == v_reg[y]) pc += 2;
        break;

    /* Vx = byte */
    case 0x6:
        v_reg[x] = byte;
        break;

    /* Vx = Vx + byte */
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

        /* Vx = Vx + Vy, VF = carry */
        case 0x4:
            v_reg[0xF] = v_reg[x] + v_reg[y] > 0xFF ? 1 : 0;
            v_reg[x] = v_reg[x] + v_reg[y];
            break;

        /* Vx = Vx - Vy, VF = !overflow */
        case 0x5:
            v_reg[0xF] = v_reg[x] > v_reg[y] ? 1 : 0;
            v_reg[x] = v_reg[x] - v_reg[y];
            break;

        /* Vx = Vy >> 1, VF = bottom bit Vy */
        case 0x6:
            v_reg[0xF] = (v_reg[y] & 0x1) == 0x1 ? 1 : 0;
            v_reg[x] = v_reg[y] >> 1;
            break;

        /* Vx = Vy - Vx, VF = !overflow */
        case 0x7:
            v_reg[0xF] = v_reg[y] > v_reg[x] ? 1 : 0;
            v_reg[x] = v_reg[y] - v_reg[x];
            break;

        /* Vx = Vy << 1, VF = top bit Vy */
        case 0xE:
            v_reg[0xF] = (v_reg[y] & 0x80) == 0x80 ? 1 : 0;
            v_reg[x] = v_reg[y] << 1;
            break;

        default:
            break;
        }
        break;

    /* skip if Vx != Vy */
    case 0x9:
        if (v_reg[x] != v_reg[y]) pc += 2;
        break;

    /* I = addr */
    case 0xA:
        i_reg = addr;
        break;

    /* jump to V0 + addr */
    case 0xB:
        pc = v_reg[0] + addr - 2;
        if (pc + 2 >= 0x1000) {
            puts("Error: program counter overflow");
            exit(EXIT_FAILURE);
        }
        break;

    /* Vx = byte & rng */
    case 0xC:
        v_reg[x] = byte & (rand() % 256);
        break;

    /* Reads 'z' byte of memory starting from I, draws sprite to 
     * (Vx, Vy) and wraps around screen. VF = collision ? 1 : 0 */
    case 0xD:
        for (row = 0; row < z; row++) {
            /* The current row of the sprite to be drawn */
            unsigned char sprite_row = ram[i_reg + row];
            for (col = 0; col < 8; col++) {
                /* Get the coords of the pixel, and xor the
                 * pixel data from sprite_row onto display. */
                int coord = ((v_reg[y] + row) * 64) + v_reg[x] + (7 - col);
                display[coord] = display[coord] != (sprite_row % 2) ? 1 : 0;
                sprite_row = sprite_row >> 1;
            }
        }
        break;

    case 0xE:
        break;

    case 0xF:
        switch (byte) {

        case 0x07:
            break;

        case 0x0A:
            break;

        case 0x15:
            break;

        case 0x18:
            break;

        case 0x1E:
            break;

        case 0x29:
            break;

        case 0x33:
            break;

        case 0x55:
            break;

        case 0x65:
            break;

        default:
            break;
        
        }
        break;

    default:
        break;
    }

    /* print_instruction(pc);
    print_v_registers();
    print_display(); */

    pc += 2;
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

void print_display() {
    int i;
    printf("dis:\t");
    for (i = 0; i < C8_DISPLAY_SIZE; i++) {
        putchar(display[i] ? '#' : '.');
        if (i % 64 == 63) { 
            putchar('\n');
            if (i != C8_DISPLAY_SIZE - 1) putchar('\t');
        }
    }
}

void print_instruction(unsigned int pc) {
    unsigned int i;
    printf("in:\t");
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
    printf("v:\t");
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
    printf("c8: v%s\n", C8_VERSION_STIRNG);
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
