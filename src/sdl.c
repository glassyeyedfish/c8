#include <SDL2/SDL.h>

#include "sdl.h"

void
poll_events(context_t* ctx) {
        int i;
        SDL_Event e;

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
                                if (e.key.keysym.sym % 1073741625 == i) {
                                        ctx->current_key_state[i] = 1;
                                }
                        }
                        break;

                case SDL_KEYUP:
                        for (i = 0; i < 512; i++) {
                                if (e.key.keysym.sym % 1073741625 == i) {
                                        ctx->previous_key_state[i] = 0;
                                }
                        }
                        break;

                default:
                        break;
                }
        }
}