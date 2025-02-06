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

# Store source assets path
set(SRC_ASSETS_PATH "${CMAKE_SOURCE_DIR}/assets")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  # Set the asset path macro to the absolute path on the dev machine and ensure SDL uses callbacks for main
  target_compile_definitions(${EXECUTABLE_NAME} PUBLIC ASSETS_PATH=\"${SRC_ASSETS_PATH}\")
else()
  # Set the asset path macro in release mode to a relative path that assumes the assets folder is in the same directory as the game executable and ensure SDL uses callbacks for main
  target_compile_definitions(${EXECUTABLE_NAME} PUBLIC ASSETS_PATH=\"assets\")
endif()
