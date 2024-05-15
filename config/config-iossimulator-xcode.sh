#!/bin/bash
cd "${0%/*}"
cd ..

mkdir -p build
mkdir -p build/ios
cd build/ios

cmake -G "Xcode" -DCMAKE_SYSTEM_NAME="iOS" -DCMAKE_OSX_SYSROOT=iphonesimulator -DCMAKE_OSX_ARCHITECTURES=x86_64 -S ../..
