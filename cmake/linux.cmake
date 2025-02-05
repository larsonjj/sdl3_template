# Set the executable name
add_executable(${EXECUTABLE_NAME})

target_sources(${EXECUTABLE_NAME} PRIVATE ${PROJECT_SOURCES})

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

# Set the asset path macro in release mode to a relative path that assumes the assets folder is in the same directory as the game executable
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_definitions(${EXECUTABLE_NAME} PUBLIC ASSETS_PATH="./assets/")
endif()
