#!/bin/bash
set -e

if [ -z "$1" ]; then
    echo "Usage: ./build.sh <filename>"
    exit 1
fi

if [ ! -f "$1.cpp" ]; then
    echo "File not found!"
    exit 1
fi

g++ -o "$1" "$1.cpp" -Ofast -march=native -frename-registers -funroll-loops -flto=2 -std=c++23 -lixwebsocket -lcrypto -lssl -lz
