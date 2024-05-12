#!/bin/bash
cd "${0%/*}"
cd ..

mkdir -p build
mkdir -p build/unix
cd build/unix

cmake --parallel --S ../..
