# Set the executable name
add_executable(${EXECUTABLE_NAME})

target_sources(${EXECUTABLE_NAME} PRIVATE ${PROJECT_SOURCES})

add_custom_command(TARGET ${EXECUTABLE_NAME} PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/assets"
)

set(CMAKE_EXECUTABLE_SUFFIX ".html")
set_target_properties(${EXECUTABLE_NAME} PROPERTIES SUFFIX ".html")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  target_compile_options(${EXECUTABLE_NAME} PRIVATE -O0)
  target_link_options(${EXECUTABLE_NAME} PRIVATE
    -s ASSERTIONS=1
    -gsource-map
    -s INITIAL_MEMORY=67108864
    -O0
    -Wall
    --preload-file assets/
    --shell-file ../../src/minshell.html
  )
else()
  target_compile_options(${EXECUTABLE_NAME} PRIVATE -Oz -flto)
  target_link_options(${EXECUTABLE_NAME} PRIVATE
    -s ASSERTIONS=0
    -s INITIAL_MEMORY=67108864
    -Oz
    -flto
    --closure 1
    -Wall
    --preload-file assets/
    --shell-file ../../src/minshell.html
  )
endif()

set(RELEASE_ASSETS_PATH "./assets/")
