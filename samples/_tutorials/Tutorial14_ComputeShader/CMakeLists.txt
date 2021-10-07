cmake_minimum_required( VERSION 3.10 )

project( Tutorial14_ComputeShader )
set( APP_TARGET "Tutorial14_ComputeShader" )

get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" ABSOLUTE )

set(SOURCES
    ${APP_PATH}/src/ComputeShaderApp.cpp
)

gx_add_sample( "${APP_TARGET}" "${APP_PATH}" "${SOURCES}" )
