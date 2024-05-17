####################################
# CMAKE Configuration
####################################

# Minimum version of CMake required to build this project
cmake_minimum_required(VERSION 3.22.1)

# Declare the project name, version, description, and languages used.
project(sdl-template
  VERSION 0.1.0
  DESCRIPTION "SDL3 Template"
  HOMEPAGE_URL "https://github.com/larsonjj/sdl_template"
  LANGUAGES C CXX
)

if (APPLE)
  # Enable Objective-C language for Apple platforms
  # Needed for SDL_Image, SDL_TTF, and SDL_Mixer
  enable_language(OBJC)
endif()

# Include FetchContent module to download dependencies
include(FetchContent)

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  # Build universal binaries on Apple Silicon
  ## NOTE: This must be set before the project() call
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
endif()

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

if (NOT WIN32)
  # Disable shared libraries
  set(BUILD_SHARED_LIBS OFF)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# Define PROJECT_INCLUDE to be the path to the include directory of the project
set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/src")

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

function(add_git_dependency libName gitURL gitTag)
  FetchContent_Declare(${libName}
    GIT_REPOSITORY ${gitURL}
    GIT_TAG ${gitTag}
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
  )
  FetchContent_MakeAvailable(${libName})
endfunction()

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/SDL/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

# Configure SDL Cmake Options
set(SDL_DIALOG OFF CACHE BOOL "" FORCE)
set(SDL_CAMERA OFF CACHE BOOL "" FORCE)
set(SDL_SENSOR OFF CACHE BOOL "" FORCE)

# Add SDL
add_subdirectory(external/SDL)
set(SDL3_FOUND TRUE)

# Add SDL Image
# option(SDL3IMAGE_BUILD_SHARED_LIBS "Build SDL3_Image as a shared library" OFF) # don't build the shared library for SDL_image
# set(DAV1D_ASM OFF CACHE BOOL "" FORCE)
# set(AOM_TARGET_CPU "generic" CACHE STRING "" FORCE)
# set(SDL3IMAGE_VENDORED "" ON)
# set(SDL3IMAGE_SAMPLES "Build SDL3_image with samples" OFF)
set(SDL3IMAGE_INSTALL OFF) # disable install target for SDL Image
# set(SDL3IMAGE_AVIF OFF)
# set(SDL3IMAGE_BMP ON)
# set(SDL3IMAGE_GIF OFF)
# set(SDL3IMAGE_JPG OFF)
# set(SDL3IMAGE_JXL OFF)
# set(SDL3IMAGE_LBM OFF)
# set(SDL3IMAGE_PCX OFF)
# set(SDL3IMAGE_PNG ON)
# set(SDL3IMAGE_PNM OFF)
# set(SDL3IMAGE_QOI OFF)
# set(SDL3IMAGE_SVG ON)
# set(SDL3IMAGE_TGA OFF)
# set(SDL3IMAGE_TIF OFF)
# set(SDL3IMAGE_WEBP ON)
# set(SDL3IMAGE_XCF OFF)
# set(SDL3IMAGE_XPM OFF)
# set(SDL3IMAGE_XV OFF)
add_git_dependency(
        SDL3_image
        https://github.com/libsdl-org/SDL_image.git
        main
)

# Add SDL Mixer
# set(SDL3MIXER_BUILD_SHARED_LIBS "Build SDL3_Mixer as a shared library" OFF)
# set(SDL3MIXER_INSTALL "" OFF) # disable install target for SDL mixer
# set(SDL3MIXER_SAMPLES "" OFF)
# set(SDL3MIXER_VENDORED "" ON)
# add_git_dependency(
#         SDL3_mixer
#         https://github.com/libsdl-org/SDL_mixer.git
#         main
# )

# Add SDL_TTF
# set(SDL3TTF_BUILD_SHARED_LIBS "Build SDL3_TTF as a shared library" OFF)
set(SDL3TTF_VENDORED "" ON)
# set(SDL3TTF_INSTALL "" OFF) # disable install target for SDL ttf
# option(SDL3TTF_SAMPLES "Build SDL3_ttf with samples" OFF)
# option(SDL2TTF_VENDORED "" ON)
add_git_dependency(
        SDL3_ttf
        https://github.com/libsdl-org/SDL_ttf.git
        main
)

####################################
# Platform Specific Configuration
####################################

### Android ###
if (ANDROID)
	# SDL java JNI interface code is hardcoded to load libmain.so on android
  # EXECUTABLE_NAME must be set to "main" for android
	set(EXECUTABLE_NAME main)
	add_library(${EXECUTABLE_NAME} SHARED)
else()
  ### MacOS ###
  if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    add_executable(${EXECUTABLE_NAME} MACOSX_BUNDLE)
  else()
    add_executable(${EXECUTABLE_NAME})
  endif()
endif()

### MacOS/iOS ###
if(APPLE)
  # Add Logo
  # target_sources("${EXECUTABLE_NAME}" PRIVATE "assets/logo.png")

  # NOTE: ios_launch_screen.storyboard is required for Apple's mobile platforms
  # It describes what to show the user while the application is starting up
  # Referenced inside Info.plist.in
  target_sources(${EXECUTABLE_NAME}
  PRIVATE
      ${PROJECT_SOURCES}
      src/ios_launch_screen.storyboard
  )

  # CPack Bundler Configuration
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    # # On macOS, make a proper .app bundle instead of a bare executable
    # BUNDLE True
    # Set the Info.plist file for Apple Mobile platforms. Without this file, your app
    # will not launch.
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist.in"
    MACOSX_BUNDLE_BUNDLE_NAME ${EXECUTABLE_NAME}
    MACOSX_BUNDLE_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
    MACOSX_BUNDLE_LONG_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
    MACOSX_BUNDLE_COPYRIGHT "Copyright ©${CURRENTYEAR} ${AUTHOR_NAME} [${AUTHOR_EMAIL}]. All rights reserved."
    MACOSX_BUNDLE_INFO_STRING "${AUTHOR_WEBSITE}"
    # MACOSX_BUNDLE_GUI_IDENTIFIER ${AUTHOR_BUNDLE_ID}
    MACOSX_BUNDLE_GUI_IDENTIFIER "$(PRODUCT_BUNDLE_IDENTIFIER)"
    # MACOSX_BUNDLE_ICON_FILE "assets/logo.png"

    # in Xcode, create a Scheme in the schemes dropdown for the app.
    XCODE_GENERATE_SCHEME TRUE

    # Custom Attributes Identification for Info.plist
    # XCODE_ATTRIBUTE_CURRENTYEAR "${CURRENTYEAR}"
  )

  ### MacOS ###
  if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    install(TARGETS ${EXECUTABLE_NAME}
        BUNDLE DESTINATION . COMPONENT Runtime
        RUNTIME DESTINATION bin COMPONENT Runtime)

    # Note Mac specific extension .app
    set(APP "\${CMAKE_INSTALL_PREFIX}/${EXECUTABLE_NAME}.app")

    # Directories to look for dependencies
    set(DIRS ${CMAKE_BINARY_DIR})

    add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>.app/../Resources/assets"
    )

    install(
    FILES
        ${PROJECT_SOURCE_DIR}/LICENSE
        ${PROJECT_SOURCE_DIR}/README.md
    DESTINATION .)

    install(CODE "include(BundleUtilities)
    fixup_bundle(\"${APP}\" \"\" \"${DIRS}\")")

    set(CPACK_GENERATOR "DRAGNDROP")
    include(CPack)
  endif()
else()
  target_sources(${EXECUTABLE_NAME} PRIVATE ${PROJECT_SOURCES})

  if (ANDROID)
    add_custom_command(TARGET ${EXECUTABLE_NAME} PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "${CMAKE_ANDROID_ASSETS_DIRECTORIES}"
      )
  else()
    add_custom_command(TARGET ${EXECUTABLE_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/assets"
          "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/assets"
      )
  endif()
endif()

### Emscripten/Web ###
if(CMAKE_SYSTEM_NAME MATCHES Emscripten)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Os")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ASSERTIONS=1 -Os -Wall --preload-file assets/ --shell-file ../../src/minshell.html")
  set_target_properties(${EXECUTABLE_NAME} PROPERTIES SUFFIX ".html")
endif()

### Windows ###
if (WIN32)
  ### Visual C++ Compiler ###
  if(MSVC)
    # Enable console for debug builds and disable it for release builds
    set_target_properties(${EXECUTABLE_NAME}
    PROPERTIES
        LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
        LINK_FLAGS_RELEASE "/SUBSYSTEM:windows /ENTRY:mainCRTStartup"
        LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:windows /ENTRY:mainCRTStartup"
        LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:windows /ENTRY:mainCRTStartup"
    )

    # Set this application as the default project
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${EXECUTABLE_NAME}")
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
  endif()
endif()

### Linux ###
if(UNIX AND NOT APPLE)
  install(
    TARGETS ${EXECUTABLE_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} # Default: bin (${c})
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} # Default: lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} # Default: lib
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # Default: include
  )

  set(APP "\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/${EXECUTABLE_NAME}")

  # Directories to look for dependencies
  set(DIRS ${CMAKE_BINARY_DIR})
  message( STATUS "DIRS: ${DIRS}")

  install(
    FILES
        ${PROJECT_SOURCE_DIR}/LICENSE
        ${PROJECT_SOURCE_DIR}/README.md
    DESTINATION .)

  install(
    DIRECTORY
        ${PROJECT_SOURCE_DIR}/assets
    DESTINATION .)

  install(CODE "include(InstallRequiredSystemLibraries)
    include(BundleUtilities)
    fixup_bundle(\"${APP}\" \"\" \"${DIRS}\")")

  set(CPACK_GENERATOR "TGZ")
  include(CPack)

endif()

####################################
# Target Configuration
####################################

# Add compiler errors/warnings flags
target_compile_options(${EXECUTABLE_NAME} PRIVATE $<$<C_COMPILER_ID:MSVC>:/W4 /WX>)
target_compile_options(${EXECUTABLE_NAME} PRIVATE $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>)

# Link dependencies

# target_link_libraries(${EXECUTABLE_NAME} PUBLIC SDL3::SDL3 SDL3_image::SDL3_image SDL3_mixer::SDL3_mixer SDL3_ttf::SDL3_ttf)
target_link_libraries(${EXECUTABLE_NAME} PUBLIC SDL3::SDL3 SDL3_image::SDL3_image SDL3_ttf::SDL3_ttf)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O0 -DDEBUG")

    # Set the asset path macro to the absolute path on the dev machine
    target_compile_definitions(${EXECUTABLE_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS ASSETS_PATH=${SRC_ASSETS_PATH})
else()
    # Set the asset path macro in release mode to a relative path that assumes the assets folder is in the same directory as the game executable
    if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
      target_compile_definitions(${EXECUTABLE_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS ASSETS_PATH="@executable_path/../Resources/assets/")
    else()
      target_compile_definitions(${EXECUTABLE_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS ASSETS_PATH="./assets/")
    endif()
endif()
