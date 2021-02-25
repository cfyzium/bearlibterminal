cmake_minimum_required(VERSION 2.8.12)

project(freetype2)

set(SOURCES ./Source/autofit/autofit.c
            ./Source/base/ftbase.c
            ./Source/base/ftbbox.c
            ./Source/base/ftdebug.c
            ./Source/base/ftfstype.c
            ./Source/base/ftgasp.c
            ./Source/base/ftglyph.c
            ./Source/base/ftinit.c
            ./Source/base/ftlcdfil.c
            ./Source/base/ftmm.c
            ./Source/base/ftsystem.c
            ./Source/raster/raster.c
            ./Source/sfnt/sfnt.c
            ./Source/smooth/smooth.c
            ./Source/truetype/truetype.c)

include_directories(./Include)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFT2_BUILD_LIBRARY")
if (UNIX)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC") 
endif()

add_library(freetype2 STATIC ${SOURCES})
