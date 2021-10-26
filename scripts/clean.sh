#!/bin/bash

for fld in $(find . -name "CMakeLists.txt")
do
    for cmakefile in CMakeCache.txt cmake_install.cmake CTestTestfile.cmake CMakeFiles Makefile
    do
        rm -rf $(dirname "$fld")/$cmakefile
    done
done
