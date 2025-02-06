# Enable Objective-C language for Apple platforms
# Needed for SDL_Image, SDL_TTF, and SDL_Mixer
enable_language(OBJC)

# Set the executable name
add_executable(${EXECUTABLE_NAME})

# Add Logo
#target_sources("${EXECUTABLE_NAME}" PRIVATE "assets/logo.png")

# NOTE: ios_launch_screen.storyboard is required for Apple's mobile platforms
# It describes what to show the user while the application is starting up
# Referenced inside Info.plist.in
target_sources(${EXECUTABLE_NAME}
PRIVATE
    ${PROJECT_SOURCES}
    src/ios_launch_screen.storyboard
)

# CPack Bundler Configuration
set_target_properties(${EXECUTABLE_NAME} PROPERTIES
  # # On macOS, make a proper .app bundle instead of a bare executable
  # BUNDLE True
  # Set the Info.plist file for Apple Mobile platforms. Without this file, your app
  # will not launch.
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/src/Info.plist.in"
  MACOSX_BUNDLE_BUNDLE_NAME ${EXECUTABLE_NAME}
  MACOSX_BUNDLE_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}"
  MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
  MACOSX_BUNDLE_LONG_VERSION_STRING "${CMAKE_PROJECT_VERSION}"
  MACOSX_BUNDLE_COPYRIGHT "Copyright ©${CURRENTYEAR} ${AUTHOR_NAME} [${AUTHOR_EMAIL}]. All rights reserved."
  MACOSX_BUNDLE_INFO_STRING "${AUTHOR_WEBSITE}"
  MACOSX_BUNDLE_GUI_IDENTIFIER ${AUTHOR_BUNDLE_ID}
  # MACOSX_BUNDLE_GUI_IDENTIFIER "$(PRODUCT_BUNDLE_IDENTIFIER)"
  # MACOSX_BUNDLE_ICON_FILE "assets/logo.png"

  # in Xcode, create a Scheme in the schemes dropdown for the app.
  XCODE_GENERATE_SCHEME TRUE

  # Custom Attributes Identification for Info.plist
  # XCODE_ATTRIBUTE_CURRENTYEAR "${CURRENTYEAR}"
)

# Instruct CMake to only search for static libraries (".a")
# set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;")

target_link_libraries(${EXECUTABLE_NAME} PRIVATE
  "-framework Foundation"
  "-framework UIKit"
  "-framework CoreGraphics"
  "-framework ImageIO"
  "-framework MobileCoreServices"
  "-framework CoreText"
)

# Copy assets to the build bundle (Only needed for local builds)
add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_SOURCE_DIR}/assets"
  "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/assets"
)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
  # Set the asset path macro to the absolute path on the dev machine and ensure SDL uses callbacks for main
  target_compile_definitions(${EXECUTABLE_NAME} PUBLIC ASSETS_PATH=${SRC_ASSETS_PATH})
else()
  # Set the asset path macro in release mode to a relative path that assumes the assets folder is in the same directory as the game executable and ensure SDL uses callbacks for main
  target_compile_definitions(${EXECUTABLE_NAME} PUBLIC ASSETS_PATH="./assets/")
endif()
