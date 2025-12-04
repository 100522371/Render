set(COMMON_SRC_FILES 
  "${CMAKE_SOURCE_DIR}/common/src/vector.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/config.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/material.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/scene.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/sphere.cpp"
  "${CMAKE_SOURCE_DIR}/common/src/cylinder.cpp"
   "${CMAKE_SOURCE_DIR}/common/src/camera.cpp"
)

set(CURRENT_DIR_SRC_FILES 
  "${CMAKE_CURRENT_SOURCE_DIR}/test_vector.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_config.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_material.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_scene.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_sphere.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_cylinder.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_camera.cpp"
)

add_unit_test_target(
  TARGET_NAME utcommon
  SOURCE_FILES ${COMMON_SRC_FILES} ${CURRENT_DIR_SRC_FILES}
  LIBRARY_FILTER common
  COVERAGE_DIR coverage-common
  LIBRARY_TO_LINK common
)
