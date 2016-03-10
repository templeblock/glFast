#!/bin/bash
cd "$(dirname -- "$(readlink -fn -- "${0}")")"

app="bin/app"

cflags="-std=c11 -g -O0 -Wpedantic -Winline -Wconversion -Wreturn-type -Wno-incompatible-pointer-types -Wno-gnu-empty-initializer"
lflags="-lSDL2 -lm ${@}"

set -x

rm -f $app

clang -o $app main.c $cflags $lflags
