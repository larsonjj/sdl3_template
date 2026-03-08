set(CMAKE_USE_PRECOMPILE_HEADERS OFF)

# Set the target name to "main" (SDL's Android code expects libmain.so)
set(EXECUTABLE_NAME main)
add_library(${EXECUTABLE_NAME} SHARED ${PROJECT_SOURCES})

get_target_property(EXEC_OUTPUT_NAME ${EXECUTABLE_NAME} OUTPUT_NAME)
message(STATUS "Executable output name is: ${EXEC_OUTPUT_NAME}")

# Set the output name explicitly as "main" (though add_library should already do this)
set_target_properties(${EXECUTABLE_NAME} PROPERTIES OUTPUT_NAME "main")

# Copy assets for Android.
add_custom_command(TARGET ${EXECUTABLE_NAME} PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "${MOBILE_ASSETS_DIR}" # Set in build.gradle
)

set(RELEASE_ASSETS_PATH "assets")
