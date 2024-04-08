cmake_minimum_required(VERSION 3.6)

project("wasteladns")

set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_CXX_STANDARD 14)

# binary output
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

if (WIN32)
# only add main.cpp as a source; other files will be found in the Solution Explorer of Visual Studio
# also add natvis file
set(SOURCES "src/main.cpp" "wasteladns.natvis")
else()
# add "src" which will be used as a reference folder
set(SOURCES "src/main.cpp" "src")
endif (WIN32)

# run cmake rules for glfw
add_subdirectory(glfw EXCLUDE_FROM_ALL)
# add glfw as a target
add_executable(app-glfw ${SOURCES})
target_compile_definitions(app-glfw PUBLIC __GLFW=1)
# link to glfw libraries
if(WIN32)
	set(LIBS-GLFW opengl32 ${GLFW_LIBRARIES})
elseif(UNIX AND NOT APPLE)
	set(LIBS-GLFW GL ${GLFW_LIBRARIES})
elseif(APPLE)
	find_library(OpenGL_LIBRARY OpenGL)
	set(LIBS-GLFW ${OpenGL_LIBRARY} ${GLFW_LIBRARIES})
endif(WIN32)
target_link_libraries(app-glfw glfw ${LIBS-GLFW})
add_custom_command(TARGET app-glfw PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:app-glfw>/assets)

# os-specific targets
if(WIN32)
	add_executable(app-dx11 WIN32 ${SOURCES})
	target_compile_definitions(app-dx11 PUBLIC __DX11=1)
	target_link_libraries(app-dx11 "d3d11.lib")
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT app-dx11)
	add_custom_command(TARGET app-dx11 PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:app-dx11>/assets)
elseif(APPLE)
	add_executable(app-macos ${SOURCES})
	target_compile_definitions(app-macos PUBLIC __MACOS=1)
	find_library(COCOA_LIBRARY Cocoa)
	target_link_libraries(app-macos ${COCOA_LIBRARY})
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY XCODE_STARTUP_PROJECT app-macos)
	# all sources need to be Objective-C++, regardless of extension
	set_target_properties(app-macos PROPERTIES XCODE_ATTRIBUTE_GCC_INPUT_FILETYPE sourcecode.cpp.objcpp)
	add_custom_command(TARGET app-macos PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:app-macos>/assets)
endif(WIN32)