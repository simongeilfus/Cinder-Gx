cmake_minimum_required( VERSION 3.10 )

# Utility function to configure samples with cinder + cinder-gx / DiligentEngine
function( gx_add_sample APP_TARGET APP_PATH SOURCES )

	get_filename_component( ASSETS_PATH "${APP_PATH}/assets" ABSOLUTE )

	include( "${CINDER_PATH}/proj/cmake/modules/cinderMakeApp.cmake" )

	ci_make_app(
	    APP_NAME    ${APP_TARGET}
	    SOURCES     ${SOURCES}
	    CINDER_PATH ${CINDER_PATH}
	    ASSETS_PATH  ${ASSETS_PATH}
	)

	get_supported_backends(DILIGENT_ENGINE_LIBRARIES)

	# from DiligentEngine\DiligentSamples\SampleBase\CMakeLists.txt
	target_link_libraries( ${APP_TARGET}
	PUBLIC
	    Diligent-PublicBuildSettings
	    Diligent-Common
	    Diligent-GraphicsTools
	    Diligent-TargetPlatform
	    ${DILIGENT_ENGINE_LIBRARIES}
	)

	target_link_libraries( ${APP_TARGET} PRIVATE cinder-gx )

	if(PLATFORM_WIN32)
	    # DiligentCore is causing /GR- flag to be added in the Release builds, we currently need this for running cinder
	    string(FIND "${CMAKE_CXX_FLAGS}" "/GR-" MSVC_HAS_NO_GR)
	    if(MSVC_HAS_NO_GR)
	        message(WARNING "CMAKE_CXX_FLAGS contains '/GR-' flag (no RTTI), removing.")
	        string(REGEX REPLACE "/GR-" "/GR" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	    endif()

	    set_target_properties(${APP_TARGET} PROPERTIES 
	        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	    )
	endif()

	copy_required_dlls( ${APP_TARGET} )

	set_target_properties( ${APP_TARGET} PROPERTIES
	    FOLDER samples
	)

endfunction()

# ----------------------------------------------------------------------
# Add tutorials

add_subdirectory( _tutorials/Tutorial01_HelloTriangle )
add_subdirectory( _tutorials/Tutorial02_Cube )
add_subdirectory( _tutorials/Tutorial03_Texturing )
add_subdirectory( _tutorials/Tutorial04_Instancing )
add_subdirectory( _tutorials/Tutorial05_TextureArray )
add_subdirectory( _tutorials/Tutorial06_Multithreading )

# add_subdirectory( _tutorials/Tutorial07_GeometryShader )
# add_subdirectory( _tutorials/Tutorial08_Tessellation )
# add_subdirectory( _tutorials/Tutorial11_ResourceUpdates )
# add_subdirectory( _tutorials/Tutorial12_RenderTarget )
# add_subdirectory( _tutorials/Tutorial13_ShadowMap )
# add_subdirectory( _tutorials/Tutorial14_ComputeShader )
# add_subdirectory( _tutorials/Tutorial18_Queries )
