#!/bin/bash
cd "$(dirname -- "$(readlink -fn -- "${0}")")"

clang++ -o app main.cpp -lSDL2 -Werror=vla -Werror=return-type -Wconversion -Wgnu-empty-initializer
