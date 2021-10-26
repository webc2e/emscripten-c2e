cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
project(unix)

add_library(
  unix STATIC 
  "File.cpp"
)
