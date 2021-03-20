cmake_minimum_required (VERSION 3.6) 
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# setup the project
project(cinder-gx)

# change runtime library
if(MSVC)
  add_compile_options($<$<CONFIG:Release>:/MT> # Runtime library: Multi-threaded
                      $<$<CONFIG:RelWithDebInfo>:/MT> # Runtime library: Multi-threaded                           
                      $<$<CONFIG:Debug>:/MTd> # Runtime library: Multi-threaded Debug
                      )
endif()

# add DiligentCore
add_subdirectory(third_party/DiligentCore)


# add sources
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
file(GLOB SOURCES "src/cinder/*.cpp" "src/cinder/graphics/*.cpp" "src/cinder/app/*.cpp" "include/cinder/*.h" "include/cinder/graphics/*.h" "include/cinder/app/*.h")
add_library(cinder-gx STATIC ${SOURCES})

# and setup include and linker settings
target_include_directories(cinder-gx PUBLIC ${INCLUDE_DIR})
target_link_libraries(cinder-gx PRIVATE
    cinder
    Diligent-GraphicsEngineVk-static
    Diligent-GraphicsEngineD3D12-static
    Diligent-GraphicsEngineD3D11-static
    Diligent-GraphicsEngineOpenGL-static
    )

# move cinder-gx into folders
set_property(TARGET cinder-gx PROPERTY FOLDER "third_party")

# move DiligentCore folder to the third_party folder
include(third_party/DiligentCore/BuildUtils.cmake)
set_directory_root_folder(third_party/DiligentCore "third_party")