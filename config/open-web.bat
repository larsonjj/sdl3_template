@echo off
setlocal
cd /d "%~dp0"
cd ..

:: Build
if not exist "build\web" mkdir "build\web"
cd build\web
call emcmake cmake ..\..
cmake --build . --config Release --parallel
cd ..\..

:: Serve and open — try emrun first, fall back to Python
set PORT=8080
set URL=http://localhost:%PORT%/sdl3_template.html

where emrun >nul 2>nul
if %errorlevel%==0 (
    emrun --port %PORT% --no_emrun_detect build\web\sdl3_template.html
    goto :eof
)

where python >nul 2>nul
if %errorlevel%==0 (
    echo Serving at %URL%
    start "" /b python -m http.server %PORT% --directory build\web
    timeout /t 1 /nobreak >nul
    start "" "%URL%"
    echo Press Ctrl+C to stop the server
    pause >nul
    goto :eof
)

echo Neither emrun nor python found. Serve build\web\ with any static file server.
exit /b 1
