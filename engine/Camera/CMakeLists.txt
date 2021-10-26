cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
project(camera)

add_library(
  camera STATIC
  MapImage.cpp
	Camera.cpp
	RemoteCamera.cpp
	MainCamera.cpp
	CameraSprite.cpp
)
