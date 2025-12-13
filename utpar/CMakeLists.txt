set(COMMON_SRC_FILES 
  "${CMAKE_SOURCE_DIR}/par/src/image_par.cpp"
  "${CMAKE_SOURCE_DIR}/par/src/render-par.cpp"
)

set(CURRENT_DIR_SRC_FILES 
  "${CMAKE_CURRENT_SOURCE_DIR}/test_image_par.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_render_par.cpp"
)

add_unit_test_target(
  TARGET_NAME utpar
  SOURCE_FILES ${COMMON_SRC_FILES} ${CURRENT_DIR_SRC_FILES}
  LIBRARY_FILTER par
  COVERAGE_DIR coverage-par
  LIBRARY_TO_LINK common
  INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/par/include
)