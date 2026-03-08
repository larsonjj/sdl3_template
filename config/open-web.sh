#!/bin/bash
set -e
cd "${0%/*}"
cd ..

# Build
mkdir -p build/web
cd build/web
emcmake cmake ../..
cmake --build ./ --config Release --parallel
cd ../..

# Serve and open — try emrun first, fall back to Python
PORT=8080
URL="http://localhost:${PORT}/sdl3_template.html"

if command -v emrun &>/dev/null; then
    emrun --port "${PORT}" --no_emrun_detect build/web/sdl3_template.html
elif command -v python3 &>/dev/null; then
    echo "Serving at ${URL}"
    python3 -m http.server "${PORT}" --directory build/web &
    SERVER_PID=$!
    sleep 1
    if [[ "$OSTYPE" == "darwin"* ]]; then
        open "${URL}"
    else
        xdg-open "${URL}" 2>/dev/null || echo "Open ${URL} in your browser"
    fi
    echo "Press Ctrl+C to stop the server"
    wait "${SERVER_PID}"
else
    echo "Neither emrun nor python3 found. Serve build/web/ with any static file server."
    exit 1
fi
