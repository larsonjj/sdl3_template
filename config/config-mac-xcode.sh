#!/bin/bash
cd "${0%/*}"
cd ..

mkdir -p build
mkdir -p build/mac

cd build/mac
cmake -G "Xcode" cmake -G "Xcode" -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO -DCMAKE_SYSTEM_NAME=Darwin -DSDL_STATIC=ON -DSDL_SHARED=OFF ../..
# cmake --build . --config Release --parallel
cmake --build . --target install --config Release --parallel

# Ensure asset folder is copied
rm -rf install/sdl_template.app/Contents/MacOS/assets || true
cp -R ../../assets install/sdl_template.app/Contents/MacOS/assets
