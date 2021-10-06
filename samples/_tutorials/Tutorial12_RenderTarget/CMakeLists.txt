cmake_minimum_required( VERSION 3.10 )

project( Tutorial12_RenderTarget )
set( APP_TARGET "Tutorial12_RenderTarget" )

get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" ABSOLUTE )

set(SOURCES
    ${APP_PATH}/src/RenderTargetApp.cpp
    ${APP_PATH}/../common/TexturedCube.cpp
)

gx_add_sample( "${APP_TARGET}" "${APP_PATH}" "${SOURCES}" )

