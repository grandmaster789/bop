# preamble
cmake_minimum_required(VERSION 3.21)

# use cmake to install vcpkg dependency management
# https://github.com/bitmeal/vcpkg-cmake-integration
set(VCPKG_VERSION latest)
include(cmake/vcpkg.cmake)

set(RootProjectName bop)
project(${RootProjectName})
enable_testing()

# try to output everything into /bin and /lib directories
set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/../lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/../bin)

foreach (OUTPUT_CONFIG ${CMAKE_CONFIGURATION_TYPES})
	string(TOUPPER ${OUTPUT_CONFIG} CURRENT_OUTPUT)

	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CURRENT_OUTPUT} ${PROJECT_BINARY_DIR}/../bin)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CURRENT_OUTPUT} ${PROJECT_BINARY_DIR}/../lib)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CURRENT_OUTPUT} ${PROJECT_BINARY_DIR}/../bin)
endforeach()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

add_subdirectory(src)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR) # detect toplevel
	add_subdirectory(tests)
endif()