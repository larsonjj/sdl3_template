#!/bin/bash
cd "${0%/*}"
cd ..

mkdir -p build
mkdir -p build/mac
cd build/mac
cmake -G "Xcode" cmake -G "Xcode" -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO -DCMAKE_SYSTEM_NAME=Darwin -DSDL_STATIC=ON -DSDL_SHARED=OFF ../..
cmake --build . --parallel
