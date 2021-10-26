#ifndef MAIN_GAME_WINDOW_H
#define MAIN_GAME_WINDOW_H

#ifndef C2E_SDL
#error This wont work
#endif

#ifdef C2E_SDL
#include "SDL/SDL_Main.h"
#else
#include "DirectX/Window.h"
#endif

#endif // MAIN_GAME_WINDOW_H
