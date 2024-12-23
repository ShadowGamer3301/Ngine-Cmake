#pragma once
#include "Core.hxx"
#include "GLFW/glfw3native.h"

namespace Ngine
{
#if defined(TARGET_PLATFORM_WINDOWS)
	class NGAPI NgineWindow;
#endif

#if !defined(TARGET_PLATFORM_XBOX)
	class NgineWindow
	{
	public:
		NgineWindow();
		~NgineWindow();

#if defined(TARGET_PLATFORM_WINDOWS)
		HWND GetWindowHandle();
#elif defined(TARGET_PLATFORM_LINUX)
		Window GetX11Window();
		Display* GetX11Display();
#endif

		bool IsFullscreen();
		uint32_t GetWidth();
		uint32_t GetHeight();

		bool UpdateWindow();

	private:
		GLFWwindow* pWindow = nullptr;
		uint32_t mSizeArray[2] = {0,0};
		bool mIsFullscreen = false;
	};
#endif

#if defined(TARGET_PLATFORM_XBOX)
	class NGAPI NgineWindow
	{
	public:
		NgineWindow();
		~NgineWindow();

		bool UpdateWindow();
	};
#endif
}