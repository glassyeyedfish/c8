#ifndef SDL_H
#define SDL_H

typedef struct {
        SDL_Window* window;
        SDL_Renderer* renderer;

        int window_should_close;
        SDL_KeyCode current_key_state[512];
        SDL_KeyCode previous_key_state[512];
} context_t;

void poll_events(context_t* ctx);

#endif