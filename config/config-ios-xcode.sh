#!/bin/bash
cd "${0%/*}"
cd ..

mkdir -p build
mkdir -p build/ios
cd build/ios

cmake -G "Xcode" -DPLATFORM="iOS" -DCMAKE_SYSTEM_NAME="iOS" -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO -DCMAKE_OSX_SYSROOT=iphoneos -DCMAKE_OSX_ARCHITECTURES=arm64 -S ../..
