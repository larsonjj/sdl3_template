@echo OFF
cd ..
mkdir build
mkdir build\web

# Ensure asset folder is copied
rd /s /q build\web\assets
xcopy /E /I assets build\web\assets

cd build\web
emcmake cmake ..\..
cmake --build . --parallel
