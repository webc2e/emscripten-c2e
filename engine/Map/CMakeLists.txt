cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
project(map)

add_library(
  map STATIC
  Map.cpp
	CARates.cpp
	MapCA.cpp
	RoomCA.cpp
)
