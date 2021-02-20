cmake_minimum_required (VERSION 3.6) 

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Generate XCode schema files
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)
# Make malloc write 0xAA to newly allocated memory and 0x55 to deallocated memory
set(CMAKE_XCODE_SCHEME_MALLOC_SCRIBBLE YES)
# Place guard pages on each side of large (4096 bytes or more) buffers
set(CMAKE_XCODE_SCHEME_MALLOC_GUARD_EDGES YES)

project(DiligentEngine)

option(DILIGENT_BUILD_TOOLS "Build DiligentTools module" ON)

add_subdirectory(third_party/DiligentCore)

if(${DILIGENT_BUILD_TOOLS})
    add_subdirectory(third_party/DiligentTools)
endif()