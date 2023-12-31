cmake_minimum_required(VERSION 3.5)

include(GNUInstallDirs)

if (UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

option(BUILD_SHARED_LIBS "Build dynamic version of library" ON)

# Detect system bitness
math(EXPR BITNESS "8*${CMAKE_SIZEOF_VOID_P}")

# Set output directory
set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/Output/${CMAKE_SYSTEM_NAME}${BITNESS})

add_subdirectory(Dependencies/FreeType)
add_subdirectory(Dependencies/PicoPNG)

project(BearLibTerminal)

if(EXISTS "../CHANGELOG.md")
	file(STRINGS "../CHANGELOG.md" TERMINAL_VERSION_TOP_LINE LIMIT_COUNT 1)
	string(REGEX MATCH "[0-9\\.]+" TERMINAL_VERSION ${TERMINAL_VERSION_TOP_LINE})
else()
	set(TERMINAL_VERSION "Unknown")
endif()


file(GLOB TERMINAL_HEADERS ./Source/*.hpp ./Source/*.h)
list(APPEND TERMINAL_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/Include/C/BearLibTerminal.h")
if(APPLE)
	file(GLOB TERMINAL_SOURCES ./Source/*.cpp ./Source/*.mm)
else()
	file(GLOB TERMINAL_SOURCES ./Source/*.cpp) # FIXME: manual selection
endif()

set(OpenGL_GL_PREFERENCE LEGACY)
find_package(OpenGL)
if (APPLE)
	find_library(COCOA_LIBRARY Cocoa)
endif()

include_directories(./Include/C)
include_directories(./Dependencies/FreeType/Include)
include_directories(./Dependencies/PicoPNG/Include)
include_directories(./Dependencies/NanoJPEG/Include)

add_library(BearLibTerminal ${TERMINAL_SOURCES} ${TERMINAL_HEADERS})
set_target_properties(BearLibTerminal PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED TRUE)

target_compile_definitions(BearLibTerminal PRIVATE TERMINAL_VERSION=\"${TERMINAL_VERSION}\")

if(BUILD_SHARED_LIBS)
	if(NOT MSVC)
		target_link_libraries(BearLibTerminal -fvisibility=hidden)
	endif()
	if(MINGW)
		target_link_libraries(BearLibTerminal -static)
	elseif(LINUX)
		target_link_libraries(BearLibTerminal -static-libgcc)
	endif()
else()
	target_compile_definitions(BearLibTerminal PRIVATE BEARLIBTERMINAL_STATIC_BUILD)
endif()

target_link_libraries(BearLibTerminal freetype2 picopng ${OPENGL_gl_LIBRARY})
target_link_libraries(BearLibTerminal picopng)
target_link_libraries(BearLibTerminal ${OPENGL_gl_LIBRARY})
if(WIN32)
	target_compile_definitions(BearLibTerminal PRIVATE UNICODE)
	target_link_libraries(BearLibTerminal winmm.lib)
	target_link_libraries(BearLibTerminal ${PROJECT_SOURCE_DIR}/Resource/Terminal-${BITNESS}bit.res)
elseif(LINUX)
	target_link_libraries(BearLibTerminal pthread)
elseif(APPLE)
	target_link_libraries(BearLibTerminal ${COCOA_LIBRARY})
endif()


set_target_properties(BearLibTerminal PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
	LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}
	ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})

if (WIN32)
	# Fix unix-like "lib" prefix and ".a" suffix
	if (BUILD_SHARED_LIBS)
		set_target_properties(BearLibTerminal PROPERTIES
			PREFIX ""
			SUFFIX ".dll"
			IMPORT_PREFIX ""
			IMPORT_SUFFIX ".lib")
	else()
		set_target_properties(BearLibTerminal PROPERTIES
			PREFIX ""
			SUFFIX ".lib")
	endif()
elseif (APPLE)
	set_target_properties(BearLibTerminal PROPERTIES
		BUILD_WITH_INSTALL_RPATH TRUE
		INSTALL_NAME_DIR "@executable_path")
endif()

install(TARGETS BearLibTerminal LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES Include/C/BearLibTerminal.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
