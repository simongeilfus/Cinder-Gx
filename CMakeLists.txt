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

# pull in cinder's exported configuration
message( "executing find_package( cinder ... )" )

if( NOT CINDER_PATH )
  get_filename_component( CINDER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cinder/" ABSOLUTE )
endif()

include( "${CINDER_PATH}/proj/cmake/configure.cmake" )

find_package( cinder REQUIRED PATHS
  "${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
  "$ENV{CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
)

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

# set common diligent directory variables
# set(DILIGENT_LIB_COMMON_DIR ${PROJECT_BINARY_DIR}/third_party/DiligentCore/Common)
# set(DILIGENT_LIB_PLATFORM_DIR ${PROJECT_BINARY_DIR}/third_party/DiligentCore/Platforms)
# set(DILIGENT_LIB_GRAPHICS_DIR ${PROJECT_BINARY_DIR}/third_party/DiligentCore/Graphics)

# # link cinder and manually link Diligent .lib files to avoid its dependencies
# target_link_libraries( cinder-gx PRIVATE cinder 
#     debug ${DILIGENT_LIB_COMMON_DIR}/Debug/Diligent-Common.lib
#     debug ${DILIGENT_LIB_PLATFORM_DIR}/Basic/Debug/Diligent-BasicPlatform.lib
#     debug ${DILIGENT_LIB_PLATFORM_DIR}/Win32/Debug/Diligent-Win32Platform.lib
#     debug ${DILIGENT_LIB_GRAPHICS_DIR}/GraphicsAccessories/Debug/Diligent-GraphicsAccessories.lib
#     debug ${DILIGENT_LIB_GRAPHICS_DIR}/GraphicsTools/Debug/Diligent-GraphicsTools.lib
  
#     optimized ${DILIGENT_LIB_COMMON_DIR}/Release/Diligent-Common.lib
#     optimized ${DILIGENT_LIB_PLATFORM_DIR}/Basic/Release/Diligent-BasicPlatform.lib
#     optimized ${DILIGENT_LIB_PLATFORM_DIR}/Win32/Release/Diligent-Win32Platform.lib
#     optimized ${DILIGENT_LIB_GRAPHICS_DIR}/GraphicsAccessories/Release/Diligent-GraphicsAccessories.lib
#     optimized ${DILIGENT_LIB_GRAPHICS_DIR}/GraphicsTools/Release/Diligent-GraphicsTools.lib
# )
# # link Diligent backend libraries
# if( NOT DILIGENT_NO_OPENGL ) 
#   target_link_libraries( cinder-gx PRIVATE 
#     debug ${DILIGENT_LIB_GRAPHICS_DIR}/GraphicsEngineOpenGL/Debug/GraphicsEngineOpenGL_64d.lib
#     optimized ${DILIGENT_LIB_GRAPHICS_DIR}/GraphicsEngineOpenGL/Release/GraphicsEngineOpenGL_64r.lib
#   )
# endif()
# if( NOT DILIGENT_NO_DIRECT3D11 ) 
#   target_link_libraries( cinder-gx PRIVATE 
#     debug ${DILIGENT_LIB_GRAPHICS_DIR}/GraphicsEngineD3D11/Debug/GraphicsEngineD3D11_64d.lib
#     optimized ${DILIGENT_LIB_GRAPHICS_DIR}/GraphicsEngineD3D11/Release/GraphicsEngineD3D11_64r.lib
#   )
# endif()
# if( NOT DILIGENT_NO_DIRECT3D12 ) 
#   target_link_libraries( cinder-gx PRIVATE 
#     debug ${DILIGENT_LIB_GRAPHICS_DIR}/GraphicsEngineD3D12/Debug/GraphicsEngineD3D12_64d.lib
#     optimized ${DILIGENT_LIB_GRAPHICS_DIR}/GraphicsEngineD3D12/Release/GraphicsEngineD3D12_64r.lib
#   )
# endif()
# if( NOT DILIGENT_NO_VULKAN ) 
#   target_link_libraries( cinder-gx PRIVATE 
#     debug ${DILIGENT_LIB_GRAPHICS_DIR}/GraphicsEngineVulkan/Debug/GraphicsEngineVk_64d.lib
#     optimized ${DILIGENT_LIB_GRAPHICS_DIR}/GraphicsEngineVulkan/Release/GraphicsEngineVk_64r.lib
#   )
# endif()

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