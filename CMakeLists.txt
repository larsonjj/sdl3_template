####################################
# CMAKE Configuration
####################################

# Minimum version of CMake required to build this project
cmake_minimum_required(VERSION 3.22.1)

# Declare the project name, version, description, and languages used.
project(sdl3_template
  VERSION 0.1.0
  DESCRIPTION "SDL3 Template"
  HOMEPAGE_URL "https://github.com/larsonjj/sdl_template"
  LANGUAGES C CXX
)

# Add CPM dependency
include(cmake/CPM.cmake)

# Set CPP standard to 20 and disable extensions
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for use with clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Ensure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Disable installing to system directories.
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

if (CMAKE_SYSTEM_NAME MATCHES "Darwin" OR CMAKE_SYSTEM_NAME MATCHES "iOS" OR CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  # Disable shared builds as we want a static binary
  set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# Define PROJECT_INCLUDE to be the path to the include directory of the project
set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/src")

# Build universal binaries on Apple Silicon
## NOTE: This must be set before the project() call
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)

# Define PROJECT_SOURCES as a list of all source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")

####################################
# Package info
####################################

# Set the name of the executable
set(EXECUTABLE_NAME ${PROJECT_NAME})

message( STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS} )
message( STATUS "CMAKE_OBJC_FLAGS: " ${CMAKE_OBJC_FLAGS} )

# Set assets path
set(SRC_ASSETS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/")

# Author Information for bundling
set(AUTHOR_NAME "John Doe")
set(AUTHOR_EMAIL "john.doe@example.com")
set(AUTHOR_WEBSITE "https://example.com")
set(AUTHOR_BUNDLE_ID "com.example.${EXECUTABLE_NAME}")
set(APP_DESCRIPTION "SDL template application")

####################################
# CPack
####################################

# Setup CPack for packaging
set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_BINARY_DIR}" "${PROJECT_NAME}" "ALL" "/")

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(CPACK_PACKAGE_CONTACT ${AUTHOR_EMAIL})
set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${AUTHOR_NAME})

####################################
# Load Dependencies
####################################

# Add SDL
set(SDL_DIALOG OFF CACHE BOOL "" FORCE)
set(SDL_CAMERA OFF CACHE BOOL "" FORCE)
set(SDL_SENSOR OFF CACHE BOOL "" FORCE)
CPMAddPackage("gh:libsdl-org/SDL#release-3.2.2")

# Add SDL Image
set(SDLIMAGE_INSTALL OFF) # disable install target for SDL Image
set(SDLIMAGE_AVIF OFF)
set(SDLIMAGE_BMP ON)
set(SDLIMAGE_GIF OFF)
set(SDLIMAGE_JPG OFF)
set(SDLIMAGE_JXL OFF)
set(SDLIMAGE_LBM OFF)
set(SDLIMAGE_PCX OFF)
set(SDLIMAGE_PNG ON)
set(SDLIMAGE_PNM OFF)
set(SDLIMAGE_QOI OFF)
set(SDLIMAGE_SVG ON)
set(SDLIMAGE_TGA OFF)
set(SDLIMAGE_TIF OFF)
set(SDLIMAGE_WEBP ON)
set(SDLIMAGE_XCF OFF)
set(SDLIMAGE_XPM OFF)
set(SDLIMAGE_XV OFF)
CPMAddPackage("gh:libsdl-org/SDL_image#release-3.2.0")

# Add SDL Mixer
set(SDLMIXER_FLAC OFF)
set(SDLMIXER_GME OFF)
set(SDLMIXER_MOD OFF)
set(SDLMIXER_MIDI OFF)
set(SDLMIXER_OPUS OFF)
CPMAddPackage("gh:libsdl-org/SDL_mixer#4870186")

# Add SDL_TTF
set(SDLTTF_VENDORED ON CACHE BOOL "" FORCE)
CPMAddPackage("gh:libsdl-org/SDL_ttf#preview-3.1.0")

####################################
# Platform Specific Configuration
####################################

### Android ###
if (CMAKE_SYSTEM_NAME MATCHES "Android")
  include(cmake/android.cmake)
elseif(CMAKE_SYSTEM_NAME MATCHES "iOS")
  include(cmake/ios.cmake)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  include(cmake/macos.cmake)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  include(cmake/linux.cmake)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
  include(cmake/windows.cmake)
elseif(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
  include(cmake/web.cmake)
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

####################################
# Target Configuration
####################################

# Add error checking flags
target_compile_options(${EXECUTABLE_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Werror>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Werror>
)

# Link dependencies
target_link_libraries(${EXECUTABLE_NAME} PUBLIC SDL3::SDL3 SDL3_image::SDL3_image SDL3_mixer::SDL3_mixer SDL3_ttf::SDL3_ttf)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O0 -DDEBUG")
endif()
