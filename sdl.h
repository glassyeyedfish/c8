#ifndef SDL_H
#define SDL_H

#define KEYCODE_MOD 1073741625 
#define KEYCODE_MAX 512

typedef struct {
        SDL_Window* window;
        SDL_Renderer* renderer;

        int window_should_close;
        SDL_KeyCode current_key_state[512];
        SDL_KeyCode previous_key_state[512];
} sdl_context_t;

void poll_events(sdl_context_t* ctx);
int is_key_down(sdl_context_t* ctx, SDL_KeyCode key);
int is_key_pressed(sdl_context_t* ctx, SDL_KeyCode key);

#endif