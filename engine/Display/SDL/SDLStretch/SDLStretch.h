// NOTE: This is under LGPL, and can't be directly linked to proprietary
// code.  It must be compiled into a separate library.  Any changes must
// be made available with binary copies of it.

// Because of this, it is compiled into a separate DLL or shared object.

/***************************************************************************
                          SDLStretch.h  -  description
                             -------------------
    begin                : Thu Nov 11 1999
    copyright            : (C) 1999 by Alexander Pipelka
    email                : pipelka@teleweb.at
 ***************************************************************************/

/***************************************************************************

     This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

 ***************************************************************************/

#ifndef INCLUDE_SDLSTRETCH
#define INCLUDE_SDLSTRETCH

#include <SDL/SDL.h>

#define sign(x) ((x) > 0 ? 1 : -1)

#ifdef _MSC_VER
// VC++
#ifndef SDL_STRETCH_DLL
// Main program
#define SDL_STRETCH_API __declspec(dllimport)
#else
// DLL
#define SDL_STRETCH_API __declspec(dllexport)
#endif
#else
#define SDL_STRETCH_API
#endif

void SDL_STRETCH_API SDL_StretchSurface(SDL_Surface *src_surface,
                                        SDL_Rect *src_rect,
                                        SDL_Surface *dst_surface,
                                        SDL_Rect *dst_rect,
                                        Uint32 *voiLUT = NULL);
void SDL_STRETCH_API SDL_StretchSurface(SDL_Surface *src_surface, int xs1,
                                        int ys1, int xs2, int ys2,
                                        SDL_Surface *dst_surface, int xd1,
                                        int yd1, int xd2, int yd2,
                                        Uint32 *voiLUT = NULL);

#endif
