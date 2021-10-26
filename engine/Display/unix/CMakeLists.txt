cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
project(unix_display)

add_library(
  unix_display STATIC 
  MemoryMappedFile.cpp
)
