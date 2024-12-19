#pragma once
#include "Core.hxx"

namespace Ngine
{
#if defined(WIN32) || defined(_WIN32)
	class NGAPI Window;
#endif

#if !defined(_GAMING_XBOX)
	class Window
	{
	public:
		Window();
		~Window();

#if defined(WIN32) || defined(_WIN32)
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
}