cmake_minimum_required(VERSION 2.8)

project(BearLibTerminal)

file(GLOB TERMINAL_SOURCES ./Source/*.cpp) # FIXME: manual selection

#set(CF "-std=c++11 -O3 -flto")
#set(LF "-flto -fvisibility=hidden")

set(CF "-std=c++11 -O3 -g0 -flto")
set(LF "-s -flto -fvisibility=hidden")

#set(CF "-std=c++11 -O0 -g3")
#set(LF "-fvisibility=hidden")

if (WIN32)
	set(CF "${CF} -DUNICODE")
	set(LF "${LF} -static")
else()
	set(LF "${LF} -static-libstdc++ -static-libgcc")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CF}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LF}")

find_package(OpenGL)

#include_directories(./include)
#include_directories(${OPENGL_INCLUDE_DIR})
include_directories(./Include)
include_directories(../Dependencies/FreeType/Include)

add_library(BearLibTerminal SHARED ${TERMINAL_SOURCES})

target_link_libraries(BearLibTerminal freetype2-minimal-static)
target_link_libraries(BearLibTerminal ${OPENGL_gl_LIBRARY})
if(WIN32)
	target_link_libraries(BearLibTerminal winmm.lib)
	target_link_libraries(BearLibTerminal ${PROJECT_SOURCE_DIR}/Resource/Terminal.res)
endif()

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/Output)
set_target_properties(BearLibTerminal PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
	LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})
	
add_custom_command(TARGET BearLibTerminal
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/Include/${PROJECT_NAME}.h ${OUTPUT_DIR})

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${OUTPUT_DIR}/${PROJECT_NAME}.h)

if (WIN32)
	# Fix unix-like "lib" prefix and ".a" suffix
	set_target_properties(BearLibTerminal PROPERTIES
		PREFIX ""
		SUFFIX ".dll"
		IMPORT_PREFIX ""
		IMPORT_SUFFIX ".lib")
endif()
