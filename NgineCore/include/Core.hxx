#pragma once

//Windows definitions
#if defined(WIN32) || defined(_WIN32) && !defined(linux)
	#if defined(_WINDLL) //If project is marked as DLL file generate additional LIB file for linking
		#define NGAPI __declspec(dllexport)
	#else
		#define NGAPI __declspec(dllimport)
	#endif

#define WIN32_LEAN_AND_MEAN //Disable windows functions that are not needed
#define GLFW_EXPOSE_NATIVE_WIN32 //Allow glfw library to interact with WINAPI

//Windows exclusive headers
#include <Windows.h>

//DirectX headers
#include <d3d11.h>
#include <d3dcompiler.h>
#include <Xinput.h>
#include <dxgi1_6.h>

//GLFW headers
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

//mINI headers
#include <mini/ini.h>

//Loguru headers
#include <loguru.hpp>

#else

#define GLFW_EXPOSE_NATIVE_X11 //Allow glfw library to interact with Wayland

//GLFW headers
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

//Vulkan headers
#include <vulkan/vulkan.hpp>

//mINI headers
#include <mini/ini.h>

//Loguru headers
#include <loguru.hpp>



#endif

//C++ stdlib headers
#include <exception>
#include <source_location>
#include <chrono>
#include <sstream>
#include <string>
#include <iostream>
#include <filesystem>
#include <array>
#include <vector>
#include <map>