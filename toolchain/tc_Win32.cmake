set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)
set(CMAKE_SYSTEM_VERSION 10.0)

if(WIN32) #If we are building on windows opt for Microsoft Compilers
	set(CMAKE_C_COMPILER "cl.exe")
	set(CMAKE_CXX_COMPILER "cl.exe")
else() #If we are building on Linux use CLang compiler
	set(CMAKE_C_COMPILER "clang")
	set(CMAKE_CXX_COMPILER "clang")
endif()