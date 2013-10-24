project(SampleOmni)

set(CMAKE_SKIP_RPATH TRUE)

set(CF "-O3 -g0")
set(LF "-s")

if(WIN32)
	set(LF "${LF} -static -mwindows")
else()
	set(LF "${LF} -static-libgcc -static-libstdc++")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CF}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LF}")

#set(SOURCE_FILES ./src/Main.cpp
#				 ./src/Common.cpp
#                 ./src/BasicOutput.cpp
#                 ./src/Fonts.cpp
#                 ./src/Speed.cpp
#                 ./src/Keyboard.cpp
#                 ./src/Mouse.cpp
#                 ./src/Sprites.cpp
#                 ./src/GetString.cpp)

set(SOURCE_FILES ./Source/Main.cpp)

include_directories(${CMAKE_SOURCE_DIR}/Terminal/Include)
add_executable(SampleOmni ${SOURCE_FILES})
target_link_libraries(SampleOmni BearLibTerminal -Wl,-R.)
if(WIN32)
	target_link_libraries(SampleOmni winmm.lib)
endif()

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/Output)
set_target_properties(SampleOmni PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
