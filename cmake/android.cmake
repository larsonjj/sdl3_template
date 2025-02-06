
# SDL java JNI interface code is hardcoded to load libmain.so on Android.
# EXECUTABLE_NAME must be set to "main" for Android.
set(EXECUTABLE_NAME main)
add_library(${EXECUTABLE_NAME} SHARED ${PROJECT_SOURCES})

# Copy assets for Android.
add_custom_command(TARGET ${EXECUTABLE_NAME} PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "${CMAKE_ANDROID_ASSETS_DIRECTORIES}"
)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  # Set the asset path macro to the absolute path on the dev machine and ensure SDL uses callbacks for main
  target_compile_definitions(${EXECUTABLE_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS=1 ASSETS_PATH=${SRC_ASSETS_PATH})
else()
  # Set the asset path macro in release mode to a relative path that assumes the assets folder is in the same directory as the game executable and ensure SDL uses callbacks for main
  target_compile_definitions(${EXECUTABLE_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS=1 ASSETS_PATH="./assets/")
endif()
