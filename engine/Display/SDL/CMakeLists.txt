cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
project(sdl)

add_subdirectory(SDLStretch)

add_library(
  sdl STATIC 
  SDL_DisplayEngine.cpp
  scrap.cpp
)
