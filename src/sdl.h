#ifndef SDL_H
#define SDL_H

#define KEYCODE_MOD 1073741625 
#define KEYCODE_MAX 512

typedef enum {
    KEY_BKSP    = 8,
    KEY_TAB     = 9,
    KEY_RETURN  = 13,
    KEY_ESC     = 27,
    KEY_SPACE   = 32,
    KEY_0       = 48,
    KEY_1, KEY_2, KEY_3,
    KEY_4, KEY_5, KEY_6,
    KEY_7, KEY_8, KEY_9,
    KEY_A       = 97,
    KEY_B, KEY_C, KEY_D, KEY_E, KEY_F,
    KEY_G, KEY_H, KEY_I, KEY_J, KEY_K,
    KEY_L, KEY_M, KEY_N, KEY_O, KEY_P,
    KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U,
    KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    KEY_RIGHT   = 278,
    KEY_LEFT, KEY_DOWN, KEY_UP
} sdl_keycode_t;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;

    int window_should_close;
    SDL_KeyCode current_key_state[512];
    SDL_KeyCode previous_key_state[512];
} sdl_context_t;

void poll_events(sdl_context_t* ctx);
int is_key_down(sdl_context_t* ctx, sdl_keycode_t key);
int is_key_pressed(sdl_context_t* ctx, sdl_keycode_t key);

#endif