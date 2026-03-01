#!/bin/bash


mkdir -p out

if [[ ! -d "raylib" ]]; then
    git clone https://github.com/raysan5/raylib
    cd raylib/src
    make PLATFORM=PLATFORM_WEB -B
    cd ../../
fi


em++ main.cpp \
    -std=c++17 \
    -Iraylib/src \
    raylib/src/libraylib.web.a \
    -s USE_GLFW=3 \
    -s ASYNCIFY \
    -s WASM=1 \
    -s MODULARIZE=1 \
    -s EXPORT_ES6=1 \
    --embed-file assets \
    -o out/index.js \
    -O3
