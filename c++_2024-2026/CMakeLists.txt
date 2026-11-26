cmake_minimum_required(VERSION 3.6)

project("wasteladns")

set(CMAKE_SUPPRESS_REGENERATION true) # no additional projects to regenerate cmake
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

if(WIN32)
	# ensure debug builds inline as needed
	set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi /Ob1 /Od /RTC1")
	# windows target using directx 11 renderer
	add_executable(app-dx11 WIN32 ${SOURCES})
	# working directory is target directory (files are relative to bin/{Configuration})
	set_target_properties(app-dx11 PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:app-dx11>")
	target_compile_definitions(app-dx11 PUBLIC __WIN64=1 __DX11=1)
	target_link_libraries(app-dx11 user32)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT app-dx11) # set as default
	add_custom_command(TARGET app-dx11 PRE_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:app-dx11>/assets)
	# windows target using opengl 3.3 renderer
	add_executable(app-gl33 WIN32 ${SOURCES})
	# working directory is target directory (files are relative to bin/{Configuration})
	set_target_properties(app-gl33 PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:app-gl33>")
	target_compile_definitions(app-gl33 PUBLIC __WIN64=1 __GL33=1)
	target_link_libraries(app-gl33 user32 gdi32)
	add_custom_command(TARGET app-gl33 PRE_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:app-gl33>/assets)
elseif(APPLE) # mac currently only has an opengl 3.3 renderer
	# ensure the project compiles with the appropiate AVX2 and FMA intrinsics
	set_source_files_properties("src/main.cpp" PROPERTIES COMPILE_OPTIONS "-march=haswell")
	# macos target using opengl 3.3 renderer
	add_executable(app-macos ${SOURCES})
	target_compile_definitions(app-macos PUBLIC __MACOS=1 __GL33=1)
	find_library(COCOA_LIBRARY Cocoa)
	find_library(IOKIT_LIBRARY IOKit)
	target_link_libraries(app-macos ${COCOA_LIBRARY})
	target_link_libraries(app-macos ${IOKIT_LIBRARY})
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY XCODE_STARTUP_PROJECT app-macos)
	# all sources need to be Objective-C++, regardless of extension
	set_target_properties(app-macos PROPERTIES XCODE_ATTRIBUTE_GCC_INPUT_FILETYPE sourcecode.cpp.objcpp)
	add_custom_command(TARGET app-macos PRE_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:app-macos>/assets)
endif(WIN32)
