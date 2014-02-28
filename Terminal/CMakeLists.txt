cmake_minimum_required(VERSION 2.8)

add_subdirectory(Dependencies/FreeType)
add_subdirectory(Dependencies/PicoPNG)

project(BearLibTerminal)

file(GLOB TERMINAL_SOURCES ./Source/*.cpp) # FIXME: manual selection

# Release flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g0")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "-s")

# Debug flags
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")

# Common flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fvisibility=hidden")
if (WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNICODE")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static")
else()
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libstdc++ -static-libgcc")
endif()

find_package(OpenGL)

include_directories(./Include/C)
include_directories(./Dependencies/FreeType/Include)
include_directories(./Dependencies/PicoPNG/Include)
include_directories(./Dependencies/NanoJPEG/Include)

add_library(BearLibTerminal SHARED ${TERMINAL_SOURCES})

target_link_libraries(BearLibTerminal freetype2)
target_link_libraries(BearLibTerminal picopng)
target_link_libraries(BearLibTerminal ${OPENGL_gl_LIBRARY})
if(WIN32)
	target_link_libraries(BearLibTerminal winmm.lib)
	math(EXPR BITNESS "8*${CMAKE_SIZEOF_VOID_P}")
	target_link_libraries(BearLibTerminal ${PROJECT_SOURCE_DIR}/Resource/Terminal-${BITNESS}bit.res)
endif()

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/Output)
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
