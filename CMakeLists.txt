cmake_minimum_required(VERSION 3.22.1)
include(FetchContent)

# Declare the project name
project(sdl_template
  VERSION 0.1.0
  DESCRIPTION "SDL3 Template"
  HOMEPAGE_URL "https://github.com/larsonjj/sdl_template"
  LANGUAGES C CXX
)

####################################
# PACKAGE INFO
####################################

# Set the name of the executable
set(EXECUTABLE_NAME ${PROJECT_NAME})

# Author
set(AUTHOR_NAME "John Doe")
set(AUTHOR_EMAIL "john.doen@example.com")
set(AUTHOR_WEBSITE "https://example.com")

# App Bundle info
set(BUNDLE_ID "com.example.${EXECUTABLE_NAME}")

####################################
# GLOBAL CMAKE CONFIGURATION
####################################
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Ensure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Disable installing to system directories.
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")

# Create an executable or a shared library based on the platform and add our sources to it
if (ANDROID)
	# The SDL java code is hardcoded to load libmain.so on android, so we need to change EXECUTABLE_NAME
	set(EXECUTABLE_NAME main)
	add_library(${EXECUTABLE_NAME} SHARED)
else()
	add_executable(${EXECUTABLE_NAME})
endif()

target_sources(${EXECUTABLE_NAME}
PRIVATE
    src/main.cpp
    src/ios_launch_screen.storyboard
)
# NOTE: ios_launch_screen.storyboard is required for Apple's mobile platforms
# It describes what to show the user while the application is starting up
# Referenced inside Info.plist.in

if(APPLE)
    target_sources("${EXECUTABLE_NAME}" PRIVATE "src/logo.png")
endif()

if(CMAKE_SYSTEM_NAME MATCHES Emscripten)
	set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
endif()

# Configure SDL by calling its CMake file.
# we use EXCLUDE_FROM_ALL so that its install targets and configs don't
# pollute upwards into our configuration.
# add_subdirectory(SDL EXCLUDE_FROM_ALL)

set(BUILD_SHARED_LIBS TRUE)
FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG main
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE # Force Raylib to find and use the SDL3 we just fetched
)
FetchContent_MakeAvailable(SDL3)

# Link SDL to our executable. This also makes its include directory available to us.
target_link_libraries(${EXECUTABLE_NAME} PUBLIC SDL3::SDL3)
target_compile_definitions(${EXECUTABLE_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS)

# set some extra configs for each platform
set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    # On macOS, make a proper .app bundle instead of a bare executable
    MACOSX_BUNDLE TRUE
    # Set the Info.plist file for Apple Mobile platforms. Without this file, your app
    # will not launch.
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist.in"
    MACOSX_BUNDLE_GUI_IDENTIFIER ${BUNDLE_ID}
    MACOSX_BUNDLE_BUNDLE_NAME ${EXECUTABLE_NAME}
    MACOSX_BUNDLE_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
    MACOSX_BUNDLE_COPYRIGHT "Copyright ©${CURRENTYEAR} ${AUTHOR_NAME} [${AUTHOR_EMAIL}]. All rights reserved."
    MACOSX_BUNDLE_INFO_STRING "${AUTHOR_WEBSITE}"
    MACOSX_BUNDLE_ICON_FILE "logo.png"

    # in Xcode, create a Scheme in the schemes dropdown for the app.
    XCODE_GENERATE_SCHEME TRUE

    # Custom Attributes Identification for Info.plist
    # XCODE_ATTRIBUTE_CURRENTYEAR "${CURRENTYEAR}"
)

# on Visual Studio, set our app as the default project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${EXECUTABLE_NAME}")

# On macOS Platforms, ensure that the bundle is valid for distribution by calling fixup_bundle.
# note that fixup_bundle does not work on iOS, so you will want to use static libraries
# or manually copy dylibs and set rpaths
# if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
#     INSTALL(CODE
#         "include(BundleUtilities)
#         fixup_bundle(\"$<TARGET_BUNDLE_DIR:${EXECUTABLE_NAME}>\" \"\" \"\")
#         "
#         COMPONENT Runtime
#     )
# endif()
# install(TARGETS ${EXECUTABLE_NAME} BUNDLE DESTINATION ./install)

