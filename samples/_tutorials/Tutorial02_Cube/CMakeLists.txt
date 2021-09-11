cmake_minimum_required( VERSION 3.10 FATAL_ERROR )
set( CMAKE_VERBOSE_MAKEFILE ON )

project( Tutorial02_Cube )
set( APP_TARGET "Tutorial02_Cube" )

message( "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

get_filename_component( CINDER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../" ABSOLUTE )
get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" ABSOLUTE )
get_filename_component( ASSETS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets" ABSOLUTE )

include( "${CINDER_PATH}/proj/cmake/modules/cinderMakeApp.cmake" )


ci_make_app(
    APP_NAME    ${APP_TARGET}
    SOURCES     ${APP_PATH}/src/CubeApp.cpp
    CINDER_PATH ${CINDER_PATH}
    ASSETS_PATH  ${ASSETS_PATH}
)

get_supported_backends(DILIGENT_ENGINE_LIBRARIES)

# from DiligentEngine\DiligentSamples\SampleBase\CMakeLists.txt
target_link_libraries( ${APP_TARGET}
PUBLIC
    Diligent-PublicBuildSettings
    Diligent-Common
    Diligent-GraphicsTools
    Diligent-TargetPlatform
    ${DILIGENT_ENGINE_LIBRARIES}
)

target_link_libraries( ${APP_TARGET} PRIVATE cinder-gx )

if(PLATFORM_WIN32)
    # DiligentCore is causing /GR- flag to be added in the Release builds, we currently need this for running cinder
    string(FIND "${CMAKE_CXX_FLAGS}" "/GR-" MSVC_HAS_NO_GR)
    if(MSVC_HAS_NO_GR)
        message(WARNING "CMAKE_CXX_FLAGS contains '/GR-' flag (no RTTI), removing.")
        string(REGEX REPLACE "/GR-" "/GR" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif()

    set_target_properties(${APP_TARGET} PROPERTIES 
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()

copy_required_dlls( ${APP_TARGET} )

set_target_properties( ${APP_TARGET} PROPERTIES
    FOLDER samples/tutorials
)