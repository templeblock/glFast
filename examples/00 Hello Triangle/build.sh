#!/bin/bash
cd "$(dirname -- "$(readlink -fn -- "${0}")")"

clang -o app main.c -std=c11 -lSDL2
