cmake_minimum_required(VERSION 2.8)

option(BUILD_SHARED_LIBS "Build dynamic version of library" ON)

add_subdirectory(Dependencies/FreeType)
add_subdirectory(Dependencies/PicoPNG)

project(BearLibTerminal)

# Detect system bitness
math(EXPR BITNESS "8*${CMAKE_SIZEOF_VOID_P}")

file(GLOB TERMINAL_HEADERS ./Source/*.hpp ./Source/*.h)
list(APPEND TERMINAL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/Include/C/BearLibTerminal.h")
if(APPLE)
	file(GLOB TERMINAL_SOURCES ./Source/*.cpp ./Source/*.mm)
else()
	file(GLOB TERMINAL_SOURCES ./Source/*.cpp) # FIXME: manual selection
endif()

# Release flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g0")
if (NOT APPLE)
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-s")
endif()

# Debug flags
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")

# Common flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fvisibility=hidden")
if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNICODE")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static")
elseif(LINUX)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc")
endif()
if (NOT BUILD_SHARED_LIBS)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBEARLIBTERMINAL_STATIC_BUILD")
endif()

find_package(OpenGL)
if (APPLE)
	find_library(COCOA_LIBRARY Cocoa)
endif()

include_directories(./Include/C)
include_directories(./Dependencies/FreeType/Include)
include_directories(./Dependencies/PicoPNG/Include)
include_directories(./Dependencies/NanoJPEG/Include)

add_library(BearLibTerminal ${TERMINAL_SOURCES} ${TERMINAL_HEADERS})

target_link_libraries(BearLibTerminal freetype2)
target_link_libraries(BearLibTerminal picopng)
target_link_libraries(BearLibTerminal ${OPENGL_gl_LIBRARY})
if(WIN32)
	target_link_libraries(BearLibTerminal winmm.lib)
	target_link_libraries(BearLibTerminal ${PROJECT_SOURCE_DIR}/Resource/Terminal-${BITNESS}bit.res)
elseif(LINUX)
	target_link_libraries(BearLibTerminal pthread)
elseif(APPLE)
	target_link_libraries(BearLibTerminal ${COCOA_LIBRARY})
endif()

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/Output/${CMAKE_SYSTEM_NAME}${BITNESS})
set_target_properties(BearLibTerminal PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
	LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})

if (WIN32)
	# Fix unix-like "lib" prefix and ".a" suffix
	set_target_properties(BearLibTerminal PROPERTIES
		PREFIX ""
		SUFFIX ".dll"
		IMPORT_PREFIX ""
		IMPORT_SUFFIX ".lib")
endif()
