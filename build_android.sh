#!/bin/bash

# Create output directory for Android builds
rm -rf build/android
mkdir -p build/android
cd build/android
cmake ../..

cd ../..

cp -f src/build.gradle build/android/_deps/sdl-src/android-project/app/build.gradle

cd build/android/_deps/sdl-src/android-project/
./gradlew assembleDebug
