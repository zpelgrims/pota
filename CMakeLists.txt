 
cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

project(lentilArnold)

set(CM_MAJOR_VERSION 1)
set(CM_MINOR_VERSION 9)
set(CM_PATCH_VERSION 0)
set(CM_VERSION "${CM_MAJOR_VERSION}.${CM_MINOR_VERSION}.${CM_PATCH_VERSION}")

set(CMAKE_VERBOSE_MAKEFILE FALSE)
set(CMAKE_SKIP_RPATH TRUE)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_CXX_STANDARD 17)


# Compiler flags
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    # Disable some of the bullshit warnings MSVC wants to barf
    add_definitions( "-W3 -D_CRT_SECURE_NO_WARNINGS -wd4005 -wd4996 -wd4305 -wd4244 -nologo" )
else()
    # Statically link stdc++ to avoid GLILBC version clashes
    set(CMAKE_SHARED_LINKER_FLAGS "-static-libstdc++")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fvisibility=hidden -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
endif()

# check if we have a local cmake include file and include it if we do
# this is useful for setting our arnold location as an alternative to
# environment variables
if(EXISTS ${CMAKE_SOURCE_DIR}/local.cmake)
    message(STATUS "Using local.cmake")
    include(${CMAKE_SOURCE_DIR}/local.cmake)
endif()

# Find Arnold SDK
find_package(Arnold REQUIRED)
include_directories(${ARNOLD_INCLUDE_DIR})
link_directories(${ARNOLD_LIBRARY_DIR})

if (NOT DEFINED INSTALL_DIR)
    if (DEFINED INSTALL_ROOT)
        set(INSTALL_DIR "${INSTALL_ROOT}/${CM_VERSION}/${ARNOLD_VERSION}")
        message("INSTALL_ROOT defined. Adding versions automatically:\n\t${INSTALL_DIR}")
    else()
        set(INSTALL_DIR "${CMAKE_BINARY_DIR}/../bin/${ARNOLD_VERSION}")
        message("INSTALL_DIR not defined. Defaulting to:\n\t${INSTALL_DIR}")
    endif()
else()
    message("Installing to:\n\t${INSTALL_DIR}")
endif()

# Set up installation paths
set(DSO_INSTALL_DIR ${INSTALL_DIR})

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/docs)

message("Shader binaries will be installed to:\n\t${DSO_INSTALL_DIR}")

# Include common files
include_directories(common)
include_directories(${CMAKE_SOURCE_DIR})

# Set the list of subdirectories to recurse into to find stuff to build
set(SUBDIRECTORIES 
    src
)

# define shader build function
function(build_shader)
    foreach(SHADER ${ARGV})
        set(SRC ${SHADER}.cpp)

        add_library(${SHADER} SHARED ${SRC})

        target_link_libraries(${SHADER} ai)
        set_target_properties(${SHADER} PROPERTIES PREFIX "")

        install(TARGETS ${SHADER} DESTINATION ${DSO_INSTALL_DIR})
    endforeach()
endfunction(build_shader)

# loop over subdirectories
foreach(SUBDIR ${SUBDIRECTORIES})
    add_subdirectory(${SUBDIR})
endforeach()

# add top-level files
INSTALL(FILES DESTINATION ${INSTALL_DIR})