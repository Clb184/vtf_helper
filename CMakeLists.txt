cmake_minimum_required(VERSION 3.5)
project(VTFWorkBench LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 20)

file(GLOB src src/*.cpp)
file(GLOB imgui_src deps/imgui/src/*.cpp)

file(GLOB vtflib_src deps/vtflib/src/*.cpp)

add_subdirectory(deps/glfw)
add_subdirectory(deps/glew/build/cmake)
add_executable(vtfworkbench ${src} ${imgui_src})

target_include_directories( vtfworkbench
	PUBLIC ${CMAKE_HOME_DIRECTORY}/deps/vtflib/lib
)

target_link_libraries(vtfworkbench
	PUBLIC glfw glew_s lib/VTFLib
)

set_target_properties(vtfworkbench
	PROPERTIES COMPILE_DEFINITIONS "GLEW_STATIC"
)

target_include_directories(vtfworkbench 
	PUBLIC
	${CMAKE_HOME_DIRECTORY}/include
	${CMAKE_HOME_DIRECTORY}/deps/glew/include
	${CMAKE_HOME_DIRECTORY}/deps/imgui/include
	${CMAKE_HOME_DIRECTORY}/deps/vtflib/include
)

message( "Dir is ${CMAKE_HOME_DIRECTORY}")
