cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
project(sdl_stretch)

add_library(sdl_stretch STATIC SDLStretch.cpp)
