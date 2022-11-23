#!/bin/bash

/opt/wasi-sdk/bin/clang++ -v -std=c++20 -fPIC \
    -L. \
    -lmongoose \
    -L/opt/wolfssl/IDE/Wasm \
    -lwolfssl \
    -I/wasm-micro-runtime/core/iwasm/libraries/lib-socket/inc \
    --target=wasm32-wasi \
    -Wl,-no-entry \
    -Wl,--export=main \
    -Wl,--export=free \
    -Wl,--export=malloc \
    -Wl,--export=realloc \
    -Wl,--export=__stack_pointer \
    -Wl,--allow-undefined \
    test.cpp -o test.wasm 

