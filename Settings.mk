# -*- Makefile -*- module defining some settings.
# $Id: Settings.mk,v 1.12 2000/06/23 15:57:43 firving Exp $

ifeq ($(COMPILER),kcc)
	CC := /usr/local/KAI/KCC.pu-4.0a-2/KCC_BASE/bin/KCC
else
	CC := em++
endif

ifeq ($(COMPILER),kcc)
	COMPILER_DESCRIPTION := Kai Compiler
else
	ifeq ($(COMPILER), pgcc)
		COMPILER_DESCRIPTION := Pentium GNU Compiler
	else
		COMPILER_DESCRIPTION := GNU Compiler
	endif
endif


CXXFLAGS := -DPIC -fPIC \
	-DC2E_SDL \
	-DC2E_NO_INLINE_ASM -DC2E_OLD_CPP_LIB \
	-DAGENT_PROFILER \
	-ftemplate-depth-32  -O2 \
	-pthread

ifeq ($(COMPILER),kcc)
	NO_OPTIMISE := +K0
else
	NO_OPTIMISE := 
endif

ASSERT     := 
NO_ASSERT  := -DNDEBUG

SYMBOLS    := -g
NO_SYMBOLS := 

PROFILE := -pg

SHAREDLINK := -s SIDE_MODULE=1 -s USE_ZLIB=1 -s USE_PTHREADS=1 -s USE_SDL=1 -s ASYNCIFY_IGNORE_INDIRECT -s ASYNCIFY

DEPENDENCYFLAGS := -M -MG

CCDEPENDENCY := g++

NASM := nasm

NASMFLAGS := -f elf


