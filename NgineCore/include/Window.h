#pragma once
#include "Core.hxx"

namespace Ngine
{
#if defined(TARGET_PLATFORM_WINDOWS)
	class NGAPI Window;
#endif

#if !defined(TARGET_PLATFORM_XBOX)
	class Window
	{
	public:
		Window();
		~Window();

#if defined(TARGET_PLATFORM_WINDOWS)
		HWND GetWindowHandle();
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
	class NGAPI Window
	{
	public:
		Window();
		~Window();

		bool UpdateWindow();
	};
#endif
}