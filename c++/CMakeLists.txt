project("wasteladns")

cmake_minimum_required(VERSION 2.8.12)

set(CMAKE_SUPPRESS_REGENERATION true)

set(CMAKE_CXX_STANDARD 14)

# binary output
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# run cmake rules for glfw
add_subdirectory(glfw)

add_executable(app-glfw "src/main.cpp" "src")
target_compile_definitions(app-glfw PUBLIC PLATFORM_GLFW=1)
#add_executable(app-directx-placeholder WIN32 "src/main.cpp" "src")
#target_compile_definitions(app-directx-placeholder PUBLIC PLATFORM_DIRECTX_SOMETHING=1)

if(WIN32)
	set(LIBS opengl32 ${GLFW_LIBRARIES})
elseif(UNIX AND NOT APPLE)
	set(LIBS GL ${GLFW_LIBRARIES})
elseif(APPLE)
	find_library(OpenGL_LIBRARY OpenGL)
	set(LIBS ${OpenGL_LIBRARY} ${GLFW_LIBRARIES})
endif(WIN32)

target_link_libraries(app-glfw glfw ${LIBS})

add_custom_command(TARGET app-glfw PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:app-glfw>/assets)
