#include "Window.h"
#include "Exception.h"
#include "FileUtils.h"

namespace Ngine
{
	Window::Window()
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

	Window::~Window()
	{
		glfwDestroyWindow(pWindow);
	}

#if defined(WIN32) || defined(_WIN32)
	HWND Window::GetWindowHandle()
	{
		return glfwGetWin32Window(pWindow);
	}
#endif

	bool Window::IsFullscreen()
	{
		return mIsFullscreen;
	}

	bool Window::UpdateWindow()
	{
		glfwPollEvents();
		return !glfwWindowShouldClose(pWindow); //Negate statement for easier usage since it originally returns true when window recived close signal
	}

	uint32_t Window::GetWidth()
	{
		return mSizeArray[0];
	}

	uint32_t Window::GetHeight()
	{
		return mSizeArray[1];
	}

	
}