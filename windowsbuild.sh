#!/bin/sh

# change this to whatever you want
CC="i686-pc-mingw32-gcc"
MAKE="make"
CFLAGS="-g -O2 -Wall -Wextra -Iwinlibs/ -Iwinlibs/SDL/"
LIBS="-Lwinlibs/ -lmingw32 -lm -llua -lz -lenet -lSDLmain -lSDL"

# ok let's roll
${MAKE} LIBS="${LIBS}" CFLAGS="${CFLAGS}" CC="${CC}" BINNAME="seabase.exe" OBJDIR="build/win32" $@

