# TestUtils.cmake - Common utilities for unit test targets

# Function to creat  if(GCOVR_EXECUTABLE)
    # Create coverage target name
    set(COVERAGE_TARGET "coverage-${PARSED_ARGS_TARGET_NAME}")
    set(COVERAGE_CLEAN_TARGET "coverage-${PARSED_ARGS_TARGET_NAME}-clean")
    
    # Use consistent naming for all targets
    
    # Coverage target that runs tests and generates reportst target with coverage support
function(add_unit_test_target)
  # Parse arguments
  cmake_parse_arguments(
    PARSED_ARGS
    ""
    "TARGET_NAME;LIBRARY_FILTER;COVERAGE_DIR;LIBRARY_TO_LINK"
    "SOURCE_FILES;INCLUDE_DIRS"
    ${ARGN}
  )
  
  # Validate required arguments
  if(NOT PARSED_ARGS_TARGET_NAME)
    message(FATAL_ERROR "TARGET_NAME is required")
  endif()
  
  if(NOT PARSED_ARGS_SOURCE_FILES)
    message(FATAL_ERROR "SOURCE_FILES is required")
  endif()
  
  if(NOT PARSED_ARGS_LIBRARY_FILTER)
    message(FATAL_ERROR "LIBRARY_FILTER is required for coverage")
  endif()
  
  # Set defaults
  if(NOT PARSED_ARGS_COVERAGE_DIR)
    set(PARSED_ARGS_COVERAGE_DIR "coverage-${PARSED_ARGS_TARGET_NAME}")
  endif()
  
  if(NOT PARSED_ARGS_LIBRARY_TO_LINK)
    set(PARSED_ARGS_LIBRARY_TO_LINK "common")
  endif()
  
  # Create the test executable
  add_executable(${PARSED_ARGS_TARGET_NAME})
  
  # Add source files
  target_sources(${PARSED_ARGS_TARGET_NAME} PRIVATE
    ${PARSED_ARGS_SOURCE_FILES}
  )
  
  # Link libraries
  target_link_libraries(${PARSED_ARGS_TARGET_NAME} 
    PRIVATE 
    ${PARSED_ARGS_LIBRARY_TO_LINK}
    Microsoft.GSL::GSL
    gtest_main
    gtest
  )
  
  # Add include directories if specified
  if(PARSED_ARGS_INCLUDE_DIRS)
    target_include_directories(${PARSED_ARGS_TARGET_NAME} PRIVATE
      ${PARSED_ARGS_INCLUDE_DIRS}
    )
  endif()
  
  # Discover and register tests with CTest
  gtest_discover_tests(${PARSED_ARGS_TARGET_NAME})
  
  # Add coverage flags
  target_compile_options(${PARSED_ARGS_TARGET_NAME} PRIVATE
    --coverage
    -fprofile-arcs
    -ftest-coverage
    -fprofile-generate
    -fprofile-update=atomic
  )
  
  # Add coverage linking flags
  target_link_options(${PARSED_ARGS_TARGET_NAME} PRIVATE
    --coverage
  )
  
  # Add coverage targets (for multi-config generators, always create but use generator expressions)
  # Find required tools
  find_program(GCOVR_EXECUTABLE gcovr)
  
  if(GCOVR_EXECUTABLE)
    # Create coverage target name
    set(COVERAGE_TARGET "coverage-${PARSED_ARGS_TARGET_NAME}")
    set(COVERAGE_CLEAN_TARGET "coverage-${PARSED_ARGS_TARGET_NAME}-clean")
    
    # Coverage target that runs tests and generates reports
      add_custom_target(${COVERAGE_TARGET}
        COMMAND ${CMAKE_COMMAND} -E echo "Running ${PARSED_ARGS_TARGET_NAME} tests to generate coverage data..."
        COMMAND $<TARGET_FILE:${PARSED_ARGS_TARGET_NAME}>
        COMMAND ${CMAKE_COMMAND} -E echo "Generating ${PARSED_ARGS_TARGET_NAME} coverage reports..."
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${PARSED_ARGS_COVERAGE_DIR}
        COMMAND ${GCOVR_EXECUTABLE} --html --html-details 
                -o ${CMAKE_BINARY_DIR}/${PARSED_ARGS_COVERAGE_DIR}/coverage_report.html
                --root ${CMAKE_SOURCE_DIR}
                --filter "${PARSED_ARGS_LIBRARY_FILTER}/"
                --exclude "out/"
                --decisions
                --calls
                --exclude-unreachable-branches
                --exclude-throw-branches
                --html-theme green
                --print-summary
        COMMAND ${GCOVR_EXECUTABLE} --xml 
                -o ${CMAKE_BINARY_DIR}/${PARSED_ARGS_COVERAGE_DIR}/coverage_report.xml
                --root ${CMAKE_SOURCE_DIR}
                --filter "${PARSED_ARGS_LIBRARY_FILTER}/"
                --exclude "out/"
                --decisions
                --calls
        COMMAND ${GCOVR_EXECUTABLE} --txt 
                -o ${CMAKE_BINARY_DIR}/${PARSED_ARGS_COVERAGE_DIR}/coverage_report.txt
                --root ${CMAKE_SOURCE_DIR}
                --filter "${PARSED_ARGS_LIBRARY_FILTER}/"
                --exclude "out/"
                --txt-metric branch
                --decisions
                --calls
        COMMAND ${CMAKE_COMMAND} -E echo "${PARSED_ARGS_TARGET_NAME} coverage reports generated in ${CMAKE_BINARY_DIR}/${PARSED_ARGS_COVERAGE_DIR}/"
        DEPENDS ${PARSED_ARGS_TARGET_NAME}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating coverage reports for ${PARSED_ARGS_TARGET_NAME}"
      )
      
      # Coverage clean target to remove coverage data
      add_custom_target(${COVERAGE_CLEAN_TARGET}
        COMMAND ${CMAKE_COMMAND} -E echo "Cleaning ${PARSED_ARGS_TARGET_NAME} coverage data..."
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/${PARSED_ARGS_COVERAGE_DIR}
        COMMAND find ${CMAKE_BINARY_DIR} -name "*.gcda" -delete
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Cleaning ${PARSED_ARGS_TARGET_NAME} coverage data files"
      )
      
    else()
      message(WARNING "gcovr not found. Coverage targets for ${PARSED_ARGS_TARGET_NAME} will not be available.")
    endif()
  
endfunction()