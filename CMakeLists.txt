cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.cmake)

project(PacNScanner
    VERSION ${PACN_VERSION}
    DESCRIPTION "Fast, modern, cross-platform local network scanner"
    HOMEPAGE_URL "https://github.com/demirgitbuh/PacNScanner"
    LANGUAGES CXX)

# --- Project-wide identity (consumed by cmake/version.h.in) ------------------
set(PACN_ORG_NAME    "PacNScanner")
set(PACN_APP_NAME    "PacNScanner")
set(PACN_GITHUB_REPO "demirgitbuh/PacNScanner")

# --- Language standard -------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# --- Options -----------------------------------------------------------------
option(PACN_BUILD_GUI   "Build the GUI application"                ON)
option(PACN_BUILD_CLI   "Build the command-line interface"         ON)
option(PACN_BUILD_TESTS "Build unit/integration tests"            ON)
option(PACN_WITH_NPCAP  "Enable optional Npcap raw/ARP scanning"   OFF)

# --- Qt ----------------------------------------------------------------------
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Network Sql)
qt_standard_project_setup()

set(PACN_HAVE_GUI FALSE)
if(PACN_BUILD_GUI)
  find_package(Qt6 6.5 REQUIRED COMPONENTS Gui Widgets Svg)
  find_package(Qt6 6.5 QUIET COMPONENTS Charts LinguistTools)
  set(PACN_HAVE_GUI TRUE)
  if(Qt6Charts_FOUND)
    message(STATUS "PacNScanner: Qt6 Charts found — rich charts enabled")
  else()
    message(STATUS "PacNScanner: Qt6 Charts NOT found — using lightweight painted charts")
  endif()
endif()

# --- Optional Npcap ----------------------------------------------------------
set(PACN_HAVE_NPCAP FALSE)
if(PACN_WITH_NPCAP AND WIN32)
  find_package(Npcap QUIET)
  if(Npcap_FOUND)
    set(PACN_HAVE_NPCAP TRUE)
    message(STATUS "PacNScanner: Npcap SDK found — raw/ARP-active scanning enabled")
  else()
    message(STATUS "PacNScanner: Npcap SDK not found — QtNetwork-only fallback")
  endif()
endif()

# --- Generated version header ------------------------------------------------
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.h.in
  ${CMAKE_BINARY_DIR}/generated/pacn/version.h @ONLY)

# --- Shared config (include roots) ------------------------------------------
add_library(pacn_config INTERFACE)
target_include_directories(pacn_config INTERFACE
  ${CMAKE_BINARY_DIR}/generated
  ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(MSVC)
  target_compile_options(pacn_config INTERFACE /utf-8 /EHsc /W4 /permissive-)
  target_compile_definitions(pacn_config INTERFACE
    NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
else()
  target_compile_options(pacn_config INTERFACE -Wall -Wextra)
endif()

# --- Modules -----------------------------------------------------------------
add_subdirectory(src/core)
add_subdirectory(src/platform)
add_subdirectory(src/storage)
add_subdirectory(src/reporting)

if(PACN_HAVE_GUI)
  add_subdirectory(src/ui)
  add_subdirectory(src/app)
endif()

if(PACN_BUILD_CLI)
  add_subdirectory(src/cli)
endif()

if(PACN_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# --- Packaging ---------------------------------------------------------------
include(cmake/packaging.cmake)
