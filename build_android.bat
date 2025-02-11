@echo off

REM Create output directory for Android builds
rmdir /s /q build\android
mkdir build\android
cd build\android
cmake ..\..

cd ..\..

copy /y src\build.gradle build\android\_deps\sdl-src\android-project\app\build.gradle

cd build\android\_deps\sdl-src\android-project\
REM gradlew clean
gradlew.bat clean
REM gradlew --info --stacktrace assembleDebug
gradlew.bat --info --stacktrace assembleDebug