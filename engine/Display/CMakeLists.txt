cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0015 NEW)
project(display)

add_subdirectory(SDL)
add_subdirectory(unix)

add_library(
  display STATIC 
  Background.cpp
	Bitmap.cpp
	ClonedGallery.cpp
	CompressedBitmap.cpp
	CreatureGallery.cpp
	DrawableObject.cpp
	DrawableObjectHandler.cpp
	ErrorMessageHandler.cpp
	Gallery.cpp
	Line.cpp
	SharedGallery.cpp
	Sprite.cpp
	System.cpp
	TintManager.cpp
	EasterEgg.cpp
	DisplayEnginePlotFunctions.cpp
  Console/ConsoleErrorDialog.cpp
)
