cmake_minimum_required( VERSION 3.10 )

project( Tutorial11_ResourceUpdates )
set( APP_TARGET "Tutorial11_ResourceUpdates" )

get_filename_component( APP_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" ABSOLUTE )

set(SOURCES
    ${APP_PATH}/src/ResourceUpdatesApp.cpp
)

gx_add_sample( "${APP_TARGET}" "${APP_PATH}" "${SOURCES}" )

