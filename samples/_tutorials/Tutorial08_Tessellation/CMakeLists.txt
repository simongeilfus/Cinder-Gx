cmake_minimum_required( VERSION 3.10 )

project( Tutorial08_Tessellation )
set( APP_TARGET "Tutorial08_Tessellation" )

get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" ABSOLUTE )

set(SOURCES
    ${APP_PATH}/src/TessellationApp.cpp
)

gx_add_sample( "${APP_TARGET}" "${APP_PATH}" "${SOURCES}" )

