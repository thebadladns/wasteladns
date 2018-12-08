project("wasteladns")

cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_SUPPRESS_REGENERATION true)

set(CMAKE_CXX_STANDARD 14)

# binary output
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# run cmake rules for glfw
add_subdirectory(glfw EXCLUDE_FROM_ALL)

if (WIN32)
# only add main; other files will be found in the Solution Explorer of Visual Studio
set(SOURCES "src/main.cpp")
else()
# add "src" which will be used as a reference folder
set(SOURCES "src/main.cpp" "src")
endif (WIN32)

add_executable(app-glfw ${SOURCES})
target_compile_definitions(app-glfw PUBLIC PLATFORM_GLFW=1)
add_executable(app-directx9 WIN32 ${SOURCES})
target_compile_definitions(app-directx9 PUBLIC PLATFORM_DIRECTX9=1)
if(${CMAKE_VERSION} VERSION_GREATER "3.6.0" OR ${CMAKE_VERSION} VERSION_EQUAL "3.6.0") 
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT app-directx9)
endif ()

if(WIN32)
	set(LIBS-GLFW opengl32 ${GLFW_LIBRARIES})
elseif(UNIX AND NOT APPLE)
	set(LIBS-GLFW GL ${GLFW_LIBRARIES})
elseif(APPLE)
	find_library(OpenGL_LIBRARY OpenGL)
	set(LIBS-GLFW ${OpenGL_LIBRARY} ${GLFW_LIBRARIES})
endif(WIN32)
target_link_libraries(app-glfw glfw ${LIBS-GLFW})

if(WIN32)
	target_link_libraries(app-directx9 "d3d9.lib")
endif(WIN32)

add_custom_command(TARGET app-glfw PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:app-glfw>/assets)
