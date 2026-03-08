# SDL3 Template

A cross-platform C game/app template built on SDL3, with SDL_image, SDL_mixer, and SDL_ttf included out of the box. Targets macOS, Windows, Linux, Web (Emscripten), iOS, and Android.

## Features

- **SDL3** — core windowing, rendering, input, and audio
- **SDL3_image** — PNG and BMP texture loading
- **SDL3_mixer** — MP3/WAV/OGG audio playback
- **SDL3_ttf** — TrueType font rendering
- **CMake + CPM** — dependency management, no manual installs
- **SDL main callbacks** — uses `SDL_AppInit` / `SDL_AppEvent` / `SDL_AppIterate` instead of a traditional `main()`
- Web loading screen with progress bar (Emscripten builds)

## Requirements

| Platform | Requirements                                                                              |
| -------- | ----------------------------------------------------------------------------------------- |
| macOS    | CMake 3.22+, Xcode or Clang                                                               |
| Windows  | CMake 3.22+, Ninja, MSVC or Clang                                                         |
| Linux    | CMake 3.22+, GCC or Clang                                                                 |
| Web      | CMake 3.22+, [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) |
| iOS      | CMake 3.22+, Xcode                                                                        |
| Android  | CMake 3.22+, Android SDK/NDK, Gradle                                                      |

## Project Structure

```
assets/          # Game assets (fonts, images, audio)
cmake/           # Platform-specific CMake includes
config/          # Build configuration scripts
src/             # Source code (main.c, platform files)
  minshell.html  # Custom Emscripten HTML shell
CMakeLists.txt   # Root CMake build file
```

## Building

### macOS (Xcode)

```bash
./config/config-mac-xcode.sh
```

Produces a `.dmg` installer in `build/mac/`.

### macOS / Linux (Unix Makefiles)

```bash
./config/config-unix.sh
```

### Windows

```bat
config\config-win.bat
```

### Web (Emscripten)

Requires the Emscripten SDK to be installed and activated (`emsdk_env.sh`).

```bash
./config/config-web-unix.sh
```

Output is placed in `build/web/`. Serve it with any static file server, e.g.:

```bash
emrun build/web/sdl3_template.html
```

### iOS (device)

```bash
./build_ios.sh
```

### iOS (simulator)

```bash
./config/config-iossimulator-xcode.sh
```

Open the generated Xcode project in `build/ios/` to run on the simulator.

### Android

```bash
./build_android.sh
```

The debug APK is output to `build/android/_deps/sdl-src/android-project/app/build/outputs/apk/debug/`.

## Customization

Before shipping, update the author/bundle details in `CMakeLists.txt`:

```cmake
set(AUTHOR_NAME    "Your Name")
set(AUTHOR_EMAIL   "you@example.com")
set(AUTHOR_WEBSITE "https://yoursite.com")
set(AUTHOR_BUNDLE_ID "com.yourcompany.${EXECUTABLE_NAME}")
set(APP_DESCRIPTION  "My awesome game")
```

## Assets

Place game assets in the `assets/` directory. In debug builds the path resolves to the source tree automatically. In release builds each platform cmake file sets `RELEASE_ASSETS_PATH` to the appropriate bundled location.

## License

See [LICENSE](LICENSE) for details.
