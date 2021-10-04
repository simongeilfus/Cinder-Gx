cmake_minimum_required( VERSION 3.10 )

project( Tutorial05_TextureArray )
set( APP_TARGET "Tutorial05_TextureArray" )

get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" ABSOLUTE )

set(SOURCES
    ${APP_PATH}/src/TextureArrayApp.cpp
    ${APP_PATH}/../common/TexturedCube.cpp
)

gx_add_sample( "${APP_TARGET}" "${APP_PATH}" "${SOURCES}" )

target_include_directories( ${APP_TARGET} PUBLIC "${APP_PATH}/../common/" )
