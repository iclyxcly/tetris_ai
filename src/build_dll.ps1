clang++ -m32 -c -o ai_dll.o ai_dll.cpp -std=c++23 -O3 -ffast-math -static -flto
clang++ -m32 -shared -o ai_dll.dll ai_dll.o -fuse-ld=lld