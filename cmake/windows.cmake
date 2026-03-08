# Set the executable name
add_executable(${EXECUTABLE_NAME})

target_sources(${EXECUTABLE_NAME} PRIVATE ${PROJECT_SOURCES})

add_custom_command(TARGET ${EXECUTABLE_NAME} PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/assets"
)

# Add std C and C++ libraries statically only on Windows
target_link_options(${EXECUTABLE_NAME} PRIVATE -static -static-libgcc)

### Visual C++ Compiler ###
if(MSVC)
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT "${EXECUTABLE_NAME}")
  set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
  if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
      add_definitions(/MP)	# parallelize each build when not using Ninja
  endif()
endif()

set(RELEASE_ASSETS_PATH "./assets/")
