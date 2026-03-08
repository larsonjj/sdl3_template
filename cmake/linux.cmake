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

# Use $ORIGIN RPATH so the binary finds shared libs relative to itself at install
set_target_properties(${EXECUTABLE_NAME} PROPERTIES
  INSTALL_RPATH "$ORIGIN"
  BUILD_WITH_INSTALL_RPATH TRUE
)

install(
  FILES
      ${PROJECT_SOURCE_DIR}/LICENSE
      ${PROJECT_SOURCE_DIR}/README.md
  DESTINATION .)

install(
  DIRECTORY
      ${PROJECT_SOURCE_DIR}/assets
  DESTINATION .)

set(CPACK_GENERATOR "TGZ")
include(CPack)

set(RELEASE_ASSETS_PATH "./assets/")
