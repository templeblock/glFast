#!/bin/bash
cd "$(dirname -- "$(readlink -fn -- "${0}")")"

c++ -o app main.cpp -lSDL2 -Werror=vla -Werror=return-type -Wconversion
