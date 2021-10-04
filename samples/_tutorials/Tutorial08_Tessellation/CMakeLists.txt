cmake_minimum_required( VERSION 3.10 )

project( Tutorial02_Cube )
set( APP_TARGET "Tutorial02_Cube" )

get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" ABSOLUTE )

set(SOURCES
    ${APP_PATH}/src/CubeApp.cpp
)

gx_add_sample( "${APP_TARGET}" "${APP_PATH}" "${SOURCES}" )

