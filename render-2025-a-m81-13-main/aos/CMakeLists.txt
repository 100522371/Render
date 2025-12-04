add_executable(render-aos)
target_sources(render-aos 
    PRIVATE 
      src/main.cpp
      src/image_aos.cpp
      src/render-aos.cpp
)
target_include_directories(render-aos PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(render-aos PRIVATE Microsoft.GSL::GSL common)