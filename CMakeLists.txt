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
target_include_directories( cinder-gx PUBLIC ${INCLUDE_DIR} )
target_compile_definitions( cinder-gx PRIVATE ENGINE_DLL=1)

target_link_libraries( cinder-gx PRIVATE cinder Diligent-Win32Platform Diligent-GraphicsAccessories Diligent-GraphicsTools )
if( NOT DILIGENT_NO_OPENGL ) 
  target_link_libraries( cinder-gx PRIVATE Diligent-GraphicsEngineOpenGL-shared )
endif()
if( NOT DILIGENT_NO_DIRECT3D11 ) 
  target_link_libraries( cinder-gx PRIVATE Diligent-GraphicsEngineD3D11-shared )
endif()
if( NOT DILIGENT_NO_DIRECT3D12 ) 
  target_link_libraries( cinder-gx PRIVATE Diligent-GraphicsEngineD3D12-shared )
endif()
if( NOT DILIGENT_NO_VULKAN ) 
  target_link_libraries( cinder-gx PRIVATE Diligent-GraphicsEngineVk-shared )
endif()

target_compile_definitions( cinder-gx PUBLIC
    D3D11_SUPPORTED=$<BOOL:${D3D11_SUPPORTED}>
    D3D12_SUPPORTED=$<BOOL:${D3D12_SUPPORTED}>
    GL_SUPPORTED=$<BOOL:${GL_SUPPORTED}>
    GLES_SUPPORTED=$<BOOL:${GLES_SUPPORTED}>
    VULKAN_SUPPORTED=$<BOOL:${VULKAN_SUPPORTED}>
    METAL_SUPPORTED=$<BOOL:${METAL_SUPPORTED}>
)

target_compile_features(cinder-gx PRIVATE cxx_std_17)

# move cinder-gx into folders
set_property( TARGET cinder-gx PROPERTY FOLDER "third_party" )

# move DiligentCore folder to the third_party folder
include(third_party/DiligentCore/BuildUtils.cmake)
set_directory_root_folder(third_party/DiligentCore "third_party")