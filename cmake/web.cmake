# Set the executable name
add_executable(${EXECUTABLE_NAME})

target_sources(${EXECUTABLE_NAME} PRIVATE ${PROJECT_SOURCES})

add_custom_command(TARGET ${EXECUTABLE_NAME} PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/assets"
)

set(CMAKE_EXECUTABLE_SUFFIX ".html")
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Os")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ASSERTIONS=1 -gsource-map -s ALLOW_MEMORY_GROWTH=1 -s MAXIMUM_MEMORY=1gb -Os -Wall --preload-file assets/ --shell-file ../../src/minshell.html")
set_target_properties(${EXECUTABLE_NAME} PROPERTIES SUFFIX ".html")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  # Set the asset path macro to the absolute path on the dev machine and ensure SDL uses callbacks for main
  target_compile_definitions(${EXECUTABLE_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS=1 ASSETS_PATH=${SRC_ASSETS_PATH})
endif()

if (CMAKE_BUILD_TYPE MATCHES "Release")
  # Set the asset path macro in release mode to a relative path that assumes the assets folder is in the same directory as the game executable and ensure SDL uses callbacks for main
  target_compile_definitions(${EXECUTABLE_NAME} PUBLIC SDL_MAIN_USE_CALLBACKS=1 ASSETS_PATH="./assets/")
endif()
