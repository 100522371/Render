cmake_minimum_required(VERSION 4.0)
project(render_project LANGUAGES CXX)

# Fetch Microsoft GSL and GoogleTest
include(FetchContent)

FetchContent_Declare(
  GSL
  GIT_REPOSITORY https://github.com/microsoft/GSL.git
  GIT_TAG        v4.1.0  # Use the latest stable version
  GIT_SHALLOW    TRUE
)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2  # Use the latest stable version
  GIT_SHALLOW    TRUE
)

# Configure GoogleTest options
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # For Windows compatibility
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)         # Don't install GoogleTest
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)           # Disable Google Mock

FetchContent_MakeAvailable(GSL googletest)

find_package(TBB REQUIRED)

# Enable testing
enable_testing()
include(GoogleTest)

# Include test utilities
include(cmake/TestUtils.cmake)

if(ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy-20 clang-tidy)
  if(CLANG_TIDY_EXE)
    message(STATUS "Found clang-tidy: ${CLANG_TIDY_EXE}")
    # Use the wrapper form so clang-tidy reads compilation database
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--extra-arg=--gcc-toolchain=/opt/gcc-14;--header-filter=.*")
  else()
    message(STATUS "clang-tidy not found; skipping CMAKE_CXX_CLANG_TIDY")
  endif()
endif()

add_subdirectory(common)
add_subdirectory(aos)
add_subdirectory(soa)
add_subdirectory(utcommon)
add_subdirectory(utaos)
add_subdirectory(utsoa)
