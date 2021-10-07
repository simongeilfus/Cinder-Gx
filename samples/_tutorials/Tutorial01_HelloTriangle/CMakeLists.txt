cmake_minimum_required( VERSION 3.10 )

project( Tutorial01_HelloTriangle )
set( APP_TARGET "Tutorial01_HelloTriangle" )

get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" ABSOLUTE )

set(SOURCES
    ${APP_PATH}/src/HelloTriangleApp.cpp
)

gx_add_sample( "${APP_TARGET}" "${APP_PATH}" "${SOURCES}" )
