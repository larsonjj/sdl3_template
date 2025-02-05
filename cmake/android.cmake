add_executable(${EXECUTABLE_NAME})

target_sources(${EXECUTABLE_NAME} PRIVATE ${PROJECT_SOURCES})

# SDL java JNI interface code is hardcoded to load libmain.so on Android.
# EXECUTABLE_NAME must be set to "main" for Android.
set(EXECUTABLE_NAME main)
add_library(${EXECUTABLE_NAME} SHARED)

# Copy assets for Android.
add_custom_command(TARGET ${EXECUTABLE_NAME} PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "${CMAKE_ANDROID_ASSETS_DIRECTORIES}"
)

# Set the asset path macro in release mode to a relative path that assumes the assets folder is in the same directory as the game executable
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_definitions(${EXECUTABLE_NAME} PUBLIC ASSETS_PATH="./assets/")
endif()
