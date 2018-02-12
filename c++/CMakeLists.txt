project("wasteladns")

cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_SUPPRESS_REGENERATION true)

# using c++11
set(CMAKE_CXX_STANDARD 14)

# binary output
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_subdirectory(glfw)

include_directories(glfw/include)
include_directories ("${PROJECT_SOURCE_DIR}/src")

add_executable(app src/Main.cpp)

if(WIN32)
	set(LIBS opengl32 ${GLFW_LIBRARIES})
elseif(UNIX AND NOT APPLE)
	set(LIBS GL ${GLFW_LIBRARIES})
elseif(APPLE)
	find_library(OpenGL_LIBRARY OpenGL)
	set(LIBS ${OpenGL_LIBRARY} ${GLFW_LIBRARIES})
endif(WIN32)

target_link_libraries(app glfw ${LIBS})

add_custom_command(TARGET app PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:app>/assets)
