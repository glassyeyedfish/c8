#include <SDL2/SDL.h>

#include "sdl.h"

void
poll_events(sdl_context_t* ctx) {
        int i;
        SDL_Event e;

        for (i = 0; i < KEYCODE_MAX; i++) {
                ctx->previous_key_state[i] = ctx->current_key_state[i];
        }

        while (SDL_PollEvent(&e)) {
                switch(e.type) {
                case SDL_WINDOWEVENT:
                        switch(e.window.event) {
                        case SDL_WINDOWEVENT_CLOSE:
                                ctx->window_should_close = 1;
                                break;
                        default:
                                break;
                        }
                        break;

                case SDL_KEYDOWN:
                        for (i = 0; i < 512; i++) {
                                if (e.key.keysym.sym % KEYCODE_MOD == i) {
                                        ctx->current_key_state[i] = 1;
                                }
                        }
                        break;

                case SDL_KEYUP:
                        for (i = 0; i < 512; i++) {
                                if (e.key.keysym.sym % KEYCODE_MOD == i) {
                                        ctx->current_key_state[i] = 0;
                                }
                        }
                        break;

                default:
                        break;
                }
        }
}

int 
is_key_down(sdl_context_t* ctx, SDL_KeyCode key) {
        return ctx->current_key_state[key % KEYCODE_MOD];
}

int 
is_key_pressed(sdl_context_t* ctx, SDL_KeyCode key) {
        return ctx->current_key_state[key % KEYCODE_MOD] 
                && !ctx->previous_key_state[key % KEYCODE_MOD];
}