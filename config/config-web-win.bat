@echo OFF
pushd ..
IF NOT EXIST build mkdir build
IF NOT EXIST build\web mkdir build\web

:: Ensure asset folder is copied
rd /s /q build\web\assets
IF %ERRORLEVEL% NEQ 0 echo "Failed to remove assets" && exit /b
xcopy /E /I assets build\web\assets
IF %ERRORLEVEL% NEQ 0 echo "Failed to copy assets" && exit /b

pushd build\web
emcmake cmake ..\..
IF %ERRORLEVEL% NEQ 0 echo "Failed to run emcmake" && exit /b
timeout /t 2 /nobreak > NUL
cmake --build . --parallel
IF %ERRORLEVEL% NEQ 0 echo "Failed to build" && exit /b
popd
popd
