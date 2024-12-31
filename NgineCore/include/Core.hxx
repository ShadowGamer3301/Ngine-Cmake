#pragma once

//Windows definitions
#if defined(TARGET_PLATFORM_WINDOWS)
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

#define DX_THROW_IF_FAILED(hr) if(FAILED(hr)) throw DirectXException(hr)
#define DX_COM_RELEASE(p) if(p != nullptr) p->Release()

#elif defined(TARGET_PLATFORM_LINUX)

#define VK_USE_PLATFORM_XLIB_KHR
#define GLFW_EXPOSE_NATIVE_X11 //Allow glfw library to interact with X11

//Xlib headers
#include <X11/X.h>

//GLFW headers
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

//Vulkan headers
#include <vulkan/vulkan.hpp>

//GLM headers
#include <glm/glm.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/trigonometric.hpp>

//mINI headers
#include <mini/ini.h>

//Loguru headers
#include <loguru.hpp>

//Crc32-11 headers
#include <crc32.hpp>

//Tga headers
#include <tga.h>

//Assimp headers
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#define VK_THROW_IF_FAILED(vkres) if(vkres != VK_SUCCESS) throw VulkanException(vkres)
#define MAX_FRAMES_IN_FLIGHT 2
#define ZeroMemory(p, size) memset(p, 0, size)

#elif defined(TARGET_PLATFORM_XBOX)

	#if defined(_WINDLL) //If project is marked as DLL file generate additional LIB file for linking
		#define NGAPI __declspec(dllexport)
	#else
		#define NGAPI __declspec(dllimport)
	#endif

//Windows headers
#include <Windows.h>

//DirectX headers
#include <d3d11.h>
#include <d3dcompiler.h>
#include <Xinput.h>

//mINI headers
#include <mini/ini.h>

//Loguru headers
#include <loguru.hpp>

#define DX_THROW_IF_FAILED(hr) if(FAILED(hr)) throw DirectXException(hr)
#define DX_COM_RELEASE(p) if(p != nullptr) p->Release()

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