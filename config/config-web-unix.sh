#!/bin/bash
cd "${0%/*}"
cd ..

mkdir -p build
mkdir -p build/web

# Ensure asset folder is copied
rm -rf build/web/assets || true
cp -R assets build/web/assets

cd build/web
emcmake cmake ../..
cmake --build . --parallel
