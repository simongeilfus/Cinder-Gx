cmake_minimum_required( VERSION 3.10 )

project( Tutorial03_Texturing )
set( APP_TARGET "Tutorial03_Texturing" )

get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" ABSOLUTE )

set(SOURCES
    ${APP_PATH}/src/TexturingApp.cpp
)

gx_add_sample( "${APP_TARGET}" "${APP_PATH}" "${SOURCES}" )
