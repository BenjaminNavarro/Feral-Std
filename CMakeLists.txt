cmake_minimum_required(VERSION 2.8)

project(Feral-Std C CXX)

set(CMAKE_CXX_STANDARD 11)
if(CMAKE_VERSION VERSION_LESS "3.1")
	add_compile_options(-std=c++11)
endif()

if(DEFINED ENV{PREFIX_DIR} AND NOT "$ENV{PREFIX_DIR}" STREQUAL "")
	set(CMAKE_INSTALL_PREFIX "$ENV{PREFIX_DIR}")
endif()
add_definitions(-DBUILD_PREFIX_DIR=${CMAKE_INSTALL_PREFIX})
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/feral")
message("-- Using PREFIX = ${CMAKE_INSTALL_PREFIX}")

# add cmake_modules path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/include/feral/Extra/cmake_modules")

set(FERAL_STD_VERSION_MAJOR 0)
set(FERAL_STD_VERSION_MINOR 0)
set(FERAL_STD_VERSION_PATCH 1)

execute_process(COMMAND date "+%a %b %d, %Y at %H:%M:%S" OUTPUT_VARIABLE BUILD_DATE OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_MACOSX_RPATH TRUE)

include(CheckCXXCompilerFlag)

# For libGMP on macOS and BSD
if(APPLE OR ${CMAKE_SYSTEM_NAME} MATCHES ".*BSD.*")
	include_directories(/usr/local/include)
endif()

find_library(GMP_LIBRARY gmp REQUIRED)
find_library(GMPXX_LIBRARY gmpxx REQUIRED)
find_package(MPFR REQUIRED)
if(NOT MPFR_FOUND)
	message(FATAL_ERROR "MPFR library is required but missing")
endif()
find_library(FERALVM_LIBRARY feralvm PATH_SUFFIXES "feral" REQUIRED)
if(NOT FERALVM_LIBRARY)
	message(FATAL_ERROR "Feral VM is required but missing ${INCLUDE_INSTALL_DIR}")
endif()

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
if(CMAKE_USE_PTHREADS_INIT)
	message("-- Using thread flags: -pthread")
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pthread")
	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -pthread")
else()
	message("-- Using thread flags: ${CMAKE_THREAD_LIBS_INIT}")
	set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${CMAKE_THREAD_LIBS_INIT}")
	set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_THREAD_LIBS_INIT}")
endif()

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES ".*BSD.*")
	add_definitions(-D_WITH_GETLINE)
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	add_definitions(-DDEBUG_MODE)
	message("-- Running in debug mode")
endif()

check_cxx_compiler_flag(-march=native COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()

# Finally! The sources!!!!!

# Install modules
install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/"
	DESTINATION "${CMAKE_INSTALL_PREFIX}/include/feral/std"
	FILES_MATCHING PATTERN "*.fer"
)

# Libraries
file(GLOB_RECURSE mods RELATIVE "${PROJECT_SOURCE_DIR}" "src/*.cpp")
foreach(m ${mods})
	get_filename_component(mod ${m} NAME_WE)
	add_library(${mod} SHARED "${m}")
	target_link_libraries(${mod} ${FERALVM_LIBRARY} ${MPFR_LIBRARIES} ${GMPXX_LIBRARY} ${GMP_LIBRARY})
	set_target_properties(${mod}
	    PROPERTIES
	    PREFIX "libferal"
	    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/feral/std"
	    INSTALL_RPATH_USE_LINK_PATH TRUE
	)
	install(TARGETS ${mod}
		LIBRARY
		  DESTINATION lib/feral/std
		  COMPONENT Libraries
	)
endforeach()