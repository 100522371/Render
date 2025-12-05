add_executable(render-soa)
target_sources(render-soa 
    PRIVATE 
      src/main.cpp
      src/image_soa.cpp
      src/render-soa.cpp
)
target_include_directories(render-soa PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(render-soa PRIVATE Microsoft.GSL::GSL common)