cmake_minimum_required(VERSION 3.12)

option(TARGET_PLATFORM_WINDOWS "Build projects for x64 Windows" OFF)
option(TARGET_PLATFORM_LINUX "Build projects for x64 GNU/Linux" OFF)
option(TARGET_PLATFORM_XBOX "Build projects for Xbox Series" OFF)

project(Loguru VERSION 2.1.0)
project(NgineCore VERSION 1.0)
project(NgineEditor VERSION 1.0)
project(NgineRuntime VERSION 1.0)

#Windows build settings
if(TARGET_PLATFORM_WINDOWS)

	#Require C++ 20 standard
	set(CMAKE_CXX_STANDARD 20)
	set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

	#Set output directories
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Win32/Lib")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Win32/Lib")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Win32/Bin")

	#Set include and link directories
	include_directories("vendor/include")
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		link_directories("vendor/lib_win_dbg")
	else()
		link_directories("vendor/lib_win_rel")
	endif()

	add_compile_definitions("WIN32" "TARGET_PLATFORM_WINDOWS")
	add_subdirectory(loguru)
	add_subdirectory(NgineCore)
	add_subdirectory(NgineRuntime)

	file(COPY "Resource" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

elseif(TARGET_PLATFORM_LINUX) #Linux build settings
	#Require C++ 20 standard
	set(CMAKE_CXX_STANDARD 20)
	set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

	#Set output directories
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Linux/Lib")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Linux/Lib")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Linux/Bin")

	#Create json file for CLangd
	set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

	add_compile_options(-fPIC)

	#Set include and link directories
	include_directories("vendor/include")
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		link_directories("vendor/lib_unix_dbg")
	else()
		link_directories("vendor/lib_unix_rel")
	endif()

	add_subdirectory(loguru)
	add_subdirectory(NgineCore)
	add_subdirectory(NgineRuntime)

	file(COPY "Resource" DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

elseif(TARGET_PLATFORM_XBOX)
	#Require C++ 20 standard
	set(CMAKE_CXX_STANDARD 20)
	set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

	#Set output directories
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Xbox/Lib")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Xbox/Lib")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Xbox/Bin")

	add_compile_definitions("TARGET_PLATFORM_XBOX")

	include_directories("$ENV{GRDKLatest}/GameKit/Include" "vendor/include")
	add_subdirectory(loguru)
	add_subdirectory(NgineCore)
	add_subdirectory(NgineRuntime)

endif()