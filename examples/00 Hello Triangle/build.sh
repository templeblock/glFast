#!/bin/bash
cd "$(dirname -- "$(readlink -fn -- "${0}")")"

clang++ -o app main.cpp -lSDL2
