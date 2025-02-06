#!/bin/bash

# Create output directory for iOS builds
mkdir -p build/ios

cmake -G "Xcode" -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_SYSROOT=iphoneos -S . -B build/ios

cmake --build build/ios --target sdl3_template --config Debug
