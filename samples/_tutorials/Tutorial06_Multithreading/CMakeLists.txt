cmake_minimum_required( VERSION 3.10 )

project( Tutorial06_Multithreading )
set( APP_TARGET "Tutorial06_Multithreading" )

get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" ABSOLUTE )

set(SOURCES
    ${APP_PATH}/src/MultithreadingApp.cpp
    ${APP_PATH}/../common/TexturedCube.cpp
)

gx_add_sample( "${APP_TARGET}" "${APP_PATH}" "${SOURCES}" )

