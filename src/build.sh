if [ -z $1 ]; then
    echo "Usage: ./build.sh <filename>"
    exit 1
fi
if [ ! -f $1.cpp ]; then
    echo "File not found!"
    exit 1
fi
clang++ -o $1 $1.cpp -Ofast -march=native -std=c++23 -lixwebsocket && ./$1
