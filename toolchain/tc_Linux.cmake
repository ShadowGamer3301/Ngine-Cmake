set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

if(WIN32) #If we are building on windows opt for CLang bundeled with Visual Studio
	set(CMAKE_C_COMPILER "clang.exe")
	set(CMAKE_CXX_COMPILER "clang.exe")
else() #If we are building on Linux use CLang compiler
	set(CMAKE_C_COMPILER "/bin/clang")
	set(CMAKE_CXX_COMPILER "/bin/clang++")
endif()

set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
