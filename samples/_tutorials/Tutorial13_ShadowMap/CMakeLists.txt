cmake_minimum_required( VERSION 3.10 )

project( Tutorial13_ShadowMap )
set( APP_TARGET "Tutorial13_ShadowMap" )

get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" ABSOLUTE )

set(SOURCES
    ${APP_PATH}/src/ShadowMapApp.cpp
    ${APP_PATH}/../common/TexturedCube.cpp
)

gx_add_sample( "${APP_TARGET}" "${APP_PATH}" "${SOURCES}" )

