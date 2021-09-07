cmake_minimum_required( VERSION 3.10 FATAL_ERROR )
set( CMAKE_VERBOSE_MAKEFILE ON )

project( Tutorial01_HelloTriangle )

message( "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

get_filename_component( CINDER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../" ABSOLUTE )
get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" ABSOLUTE )

include( "${CINDER_PATH}/proj/cmake/modules/cinderMakeApp.cmake" )

ci_make_app(
    APP_NAME    "Tutorial01_HelloTriangle"
    SOURCES     ${APP_PATH}/src/HelloTriangleApp.cpp
    CINDER_PATH ${CINDER_PATH}
)

# TODO: set APP_NAME as a variable
#target_include_directories( "Tutorial01_HelloTriangle" PUBLIC ${ARG_INCLUDES} )
#target_link_libraries( "Tutorial01_HelloTriangle" cinder ${ARG_LIBRARIES} )

get_supported_backends(DILIGENT_ENGINE_LIBRARIES)

# from DiligentEngine\DiligentSamples\SampleBase\CMakeLists.txt
target_link_libraries(Tutorial01_HelloTriangle
PRIVATE 
    Diligent-BuildSettings
PUBLIC
    Diligent-Common
    Diligent-GraphicsTools
    # Diligent-TextureLoader
    Diligent-TargetPlatform
    # Diligent-Imgui
    ${DILIGENT_ENGINE_LIBRARIES}
    # Diligent-NativeAppBase
)

target_link_libraries( Tutorial01_HelloTriangle PRIVATE cinder-gx )

copy_required_dlls( Tutorial01_HelloTriangle )

set_target_properties( Tutorial01_HelloTriangle PROPERTIES
    FOLDER samples/tutorials
)
