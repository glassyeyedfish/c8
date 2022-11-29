#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#include "sdl.h"

#define C8_VERSION_STIRNG "0.1.0"

#define C8_FONT_DATA_SIZE (16 * 5)
#define C8_DISPLAY_SIZE (64 * 32)

#define C8_MAX_CALL_STACK (16)

sdl_context_t ctx = { 0 };

int flag_stepping = 0;

unsigned short pc = 0x200;
unsigned char ram[0x1000] = { 0 };

unsigned char v_reg[16] = { 0 };
unsigned short i_reg = 0;

unsigned short sp = 0;
unsigned short call_stack[C8_MAX_CALL_STACK] = { 0 };

unsigned char delay_timer = 0;
unsigned char sound_timer = 0;

unsigned char display[C8_DISPLAY_SIZE] = { 0 };

unsigned char keyboard = 0;

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
void print_i_register(void);
void print_instruction(unsigned int pc);
void print_stack(void);
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
    unsigned long t = 0;
    unsigned long dt = SDL_GetTicks64();

    /* Initialize everything. */
    init_rom();

    /* Main loop. */
    while (!ctx.window_should_close) {
        /* Event handling. */
        poll_events(&ctx);

        /* 60Hz timer */
        dt = SDL_GetTicks64() - dt;
        t += dt;

        if (t > 16) {
            t = 0;
            if (delay_timer > 0) delay_timer--;
            if (sound_timer > 0) sound_timer--;

            /* Update diplay. */
            SDL_RenderClear(ctx.renderer);
            for (i = 0; i < C8_DISPLAY_SIZE; i++) {
                unsigned int color = display[i] != 0 ? 255 : 0;
                SDL_SetRenderDrawColor(ctx.renderer, color, color, color, 255);
                SDL_RenderDrawPoint(ctx.renderer, i % 64, (int) i / 64);
            }
            SDL_RenderPresent(ctx.renderer);
        }

        /* Update keyboard data structure. */
        keyboard = 0x10;

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

        if (is_key_down(&ctx, KEY_1)) keyboard = 0x1;
        else if (is_key_down(&ctx, KEY_2)) keyboard = 0x2;
        else if (is_key_down(&ctx, KEY_3)) keyboard = 0x3;
        else if (is_key_down(&ctx, KEY_4)) keyboard = 0xC;
        else if (is_key_down(&ctx, KEY_Q)) keyboard = 0x4;
        else if (is_key_down(&ctx, KEY_W)) keyboard = 0x5;
        else if (is_key_down(&ctx, KEY_E)) keyboard = 0x6;
        else if (is_key_down(&ctx, KEY_R)) keyboard = 0xD;
        else if (is_key_down(&ctx, KEY_A)) keyboard = 0x7;
        else if (is_key_down(&ctx, KEY_S)) keyboard = 0x8;
        else if (is_key_down(&ctx, KEY_D)) keyboard = 0x9;
        else if (is_key_down(&ctx, KEY_F)) keyboard = 0xE;
        else if (is_key_down(&ctx, KEY_Z)) keyboard = 0xA;
        else if (is_key_down(&ctx, KEY_X)) keyboard = 0x0;
        else if (is_key_down(&ctx, KEY_C)) keyboard = 0xB;
        else if (is_key_down(&ctx, KEY_V)) keyboard = 0xF;

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

        /* SDL_Delay(1); */
    }


    /* Clean up */
    SDL_DestroyRenderer(ctx.renderer);
    SDL_DestroyWindow(ctx.window);

    SDL_Quit();
}

void
eval_instruction(void) {
    unsigned int i, j;

    /* Seperate instruction into important pieces. */
    unsigned int inst = (ram[pc] << 8) | ram[pc + 1];
    unsigned int addr = inst & 0x0FFF;
    unsigned int byte = inst & 0x00FF;
    unsigned int w = (inst & 0xF000) >> 12;
    unsigned int x = (inst & 0x0F00) >> 8;
    unsigned int y = (inst & 0x00F0) >> 4;
    unsigned int z = inst & 0x000F;

    print_instruction(pc);

    /* Increment program counter */
    pc += 2;

    /* Switch over specific nibbles to determine which 
     * instruction should be executed, then exec it. */
    switch(w) {

    case 0x0:
        switch (byte) {

        case 0xE0:
            for (i = 0; i < C8_DISPLAY_SIZE; i++) {
                display[i] = 0;
            }
            break;

        case 0xEE:
            if (sp <= 0) {
                puts("Error: stack underflow");
                exit(EXIT_FAILURE);
            }
            sp--;
            pc = call_stack[sp];
            break;

        }
        break;

    /* jump to addr */
    case 0x1:
        pc = addr;
        break;
        
    /* call subroutine at addr */
    case 0x2:
        if (sp >= C8_MAX_CALL_STACK) {
            puts("Error: stack overflow");
            exit(EXIT_FAILURE);
        }
        call_stack[sp] = pc;
        sp++;
        pc = addr;
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
            v_reg[0xF] = v_reg[y] > v_reg[x] ? 0 : 1;
            v_reg[x] = v_reg[x] - v_reg[y];
            break;

        /* Vx = Vy >> 1, VF = bottom bit Vy */
        case 0x6:
            v_reg[0xF] = v_reg[y] && 0x1;
            v_reg[x] = v_reg[y] >> 1;
            break;

        /* Vx = Vy - Vx, VF = !overflow */
        case 0x7:
            v_reg[0xF] = v_reg[x] > v_reg[y] ? 0 : 1;
            v_reg[x] = v_reg[y] - v_reg[x];
            break;

        /* Vx = Vy << 1, VF = top bit Vy */
        case 0xE:
            v_reg[0xF] = (v_reg[y] >> 7) & 0x1;
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
        pc = v_reg[0] + addr;
        if (pc + 2 >= 0x1000) {
            puts("Error: program counter overflow");
            exit(EXIT_FAILURE);
        }
        break;

    /* Vx = byte & rng */
    case 0xC:
        v_reg[x] = byte & (rand() % 0x100);
        break;

    /* Reads 'z' byte of memory starting from I, draws sprite to 
     * (Vx, Vy) and wraps around screen. VF = collision ? 1 : 0 */
    case 0xD:
        for (i = 0; i < z; i++) {
            /* The current i of the sprite to be drawn */
            unsigned char sprite_i = ram[i_reg + i];
            for (j = 0; j < 8; j++) {
                /* Get the coords of the pixel, and xor the
                 * pixel data from sprite_i onto display. */
                int coord = ((v_reg[y] + i) * 64) + v_reg[x] + (7 - j);
                display[coord] = display[coord] != (sprite_i % 2) ? 1 : 0;
                sprite_i = sprite_i >> 1;
            }
        }
        break;

    case 0xE:
        switch (byte) {

        case 0x9E:
            if (keyboard != 0x10 && v_reg[x] == keyboard) pc += 2;
            break;

        case 0xA1:
            if (keyboard != 0x10 && v_reg[x] != keyboard) pc += 2;
            break;

        default:
            break;
        }
        break;

    case 0xF:
        switch (byte) {

        case 0x07:
            v_reg[x] = delay_timer;
            break;

        case 0x0A:
            if (keyboard == 0x10) pc -= 2;
            else v_reg[x] = keyboard;
            break;

        case 0x15:
            delay_timer = v_reg[x];
            break;

        case 0x18:
            sound_timer = v_reg[x];
            break;

        case 0x1E:
            i_reg += v_reg[x];
            break;

        case 0x29:
            i_reg = v_reg[x] * 5;
            break;

        case 0x33:
            ram[i_reg + 0] = (v_reg[x] / 100) % 10;
            ram[i_reg + 1] = (v_reg[x] / 10) % 10;
            ram[i_reg + 2] = (v_reg[x] / 1) % 10;
            break;

        case 0x55:
            for (i = 0; i <= x; i++) {
                ram[i_reg + i] = v_reg[i];
                i_reg += x + 1;
            }
            break;

        case 0x65:
            for (i = 0; i <= x; i++) {
                v_reg[i] = ram[i_reg + i];
                i_reg += x + 1;
            }
            break;

        default:
            break;
        
        }
        break;

    default:
        break;
    }

    print_i_register(); putchar('\n');
    print_v_registers(); putchar('\n');
    print_stack(); putchar('\n');
    printf("RAM @ [0x%x]: 0x%x\n", 0x100, ram[0x100]);
    printf("RAM @ [0x%x]: 0x%x\n", 0x101, ram[0x101]);
    printf("RAM @ [0x%x]: 0x%x\n", 0x102, ram[0x102]);

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
    printf("in:\t");
    for (i = pc; i <= pc + 1; i++) {
        if (ram[i] < 0x10) {
            putchar('0');
        }
        printf("%x", ram[i]);
    }
    printf("\t\t@ 0x%x", pc);
}

void print_i_register(void) {
    printf("\t\tI = 0x%x", i_reg);
}

void print_stack() {
    unsigned int i;
    printf("st:\t[");
    for (i = 0; i <= sp; i++) {
        printf(" 0x%x", call_stack[i]);
    }
}

void print_v_registers(void) {
    int i;
    printf("v:\t");
    for (i = 0; i < 16; i++) {
        printf("V%x = %x ", i, v_reg[i]);
    }
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
