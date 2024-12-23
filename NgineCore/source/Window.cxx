#include "Window.h"
#include "Exception.h"
#include "FileUtils.h"
#include "GLFW/glfw3native.h"

namespace Ngine
{
#if !defined(TARGET_PLATFORM_XBOX)
	NgineWindow::NgineWindow()
	{
		if (!glfwInit())
			throw Exception();

		mIsFullscreen = FileUtils::GetBoolFromConfig("Resource/ngine.ini", "General", "WindowFullscreen");
		mSizeArray[0] = FileUtils::GetIntegerFromConfig("Resource/ngine.ini", "General", "WindowWidth");
		mSizeArray[1] = FileUtils::GetIntegerFromConfig("Resource/ngine.ini", "General", "WindowHeight");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		if(mIsFullscreen)
			pWindow = glfwCreateWindow(mSizeArray[0], mSizeArray[1], "NgineRuntime", glfwGetPrimaryMonitor(), nullptr);
		else
			pWindow = glfwCreateWindow(mSizeArray[0], mSizeArray[1], "NgineRuntime", nullptr, nullptr);

		if (!pWindow)
			throw Exception();
	}

	NgineWindow::~NgineWindow()
	{
		glfwDestroyWindow(pWindow);
	}

#if defined(TARGET_PLATFORM_WINDOWS)
	HWND NgineWindow::GetWindowHandle()
	{
		return glfwGetWin32Window(pWindow);
	}
#elif defined(TARGET_PLATFORM_LINUX)
	Window NgineWindow::GetX11Window()
	{
		return glfwGetX11Window(pWindow);
	}

	Display* NgineWindow::GetX11Display()
	{
		return glfwGetX11Display();
	}
#endif

	bool NgineWindow::IsFullscreen()
	{
		return mIsFullscreen;
	}

	bool NgineWindow::UpdateWindow()
	{
		glfwPollEvents();
		return !glfwWindowShouldClose(pWindow); //Negate statement for easier usage since it originally returns true when window recived close signal
	}

	uint32_t NgineWindow::GetWidth()
	{
		return mSizeArray[0];
	}

	uint32_t NgineWindow::GetHeight()
	{
		return mSizeArray[1];
	}

#else

	NgineWindow::NgineWindow()
	{

	}

	NgineWindow::~NgineWindow()
	{

	}

	bool NgineWindow::UpdateWindow()
	{
		return true;
	}

#endif
}