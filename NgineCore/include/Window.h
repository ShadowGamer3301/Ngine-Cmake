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
	private:
		class WindowClass
		{
		public:
			static const char* GetName() noexcept;
			static HINSTANCE GetInstance() noexcept;
		private:
			WindowClass() noexcept;
			~WindowClass();
			WindowClass(const WindowClass&) = delete;
			WindowClass& operator=(const WindowClass&) = delete;
			static constexpr const char* wndClassName = "Ngine Xbox Window";
			static WindowClass wndClass;
			HINSTANCE hInst;
		};

	public:
		NgineWindow();
		~NgineWindow();

		bool UpdateWindow();
		HWND GetWindowHandle();

		uint32_t GetWidth();
		uint32_t GetHeight();

	private:
		static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

	private:
		HWND hWnd;
		uint32_t mSizeArray[2] = { 0,0 };
	};
#endif
}