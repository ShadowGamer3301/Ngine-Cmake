#include "Window.h"
#include "Exception.h"
#include "FileUtils.h"
#include "Event.h"

namespace Ngine
{
#if !defined(TARGET_PLATFORM_XBOX)
	void NgineWindow::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
		EventCursorMove* pEvent = new EventCursorMove();

		pEvent->pos_x = xpos;
		pEvent->pos_y = ypos;
		pEvent->mType = EventType::EventType_CursorMove;

		EventHandler::AddEventToBuffer(pEvent, EventType::EventType_CursorMove);
		
	}

	void NgineWindow::KeyInputCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		EventKeyAction* pEvent = new EventKeyAction();

		pEvent->mKey = key;
		pEvent->mPressed = (action == GLFW_PRESS || action == GLFW_REPEAT);
		pEvent->mType = EventType::EventType_KeyAction;

		EventHandler::AddEventToBuffer(pEvent, EventType::EventType_KeyAction);
	}

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

		glfwSetKeyCallback(pWindow, KeyInputCallback);
		glfwSetCursorPosCallback(pWindow, CursorPosCallback);
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

	NgineWindow::WindowClass NgineWindow::WindowClass::wndClass;

	NgineWindow::WindowClass::WindowClass() noexcept
		: hInst(GetModuleHandle(nullptr))
	{
		WNDCLASSEX wcex = {};
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = HandleMsgSetup;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = GetInstance();
		wcex.hIcon = nullptr;
		wcex.hCursor = nullptr;
		wcex.hbrBackground = nullptr;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = GetName();
		wcex.hIconSm = nullptr;

		if (!RegisterClassEx(&wcex))
		{
			LOG_F(ERROR, "Failed to register window class");
			throw Exception();
		}
	}

	NgineWindow::WindowClass::~WindowClass()
	{
		UnregisterClass(GetName(), GetInstance());
	}

	const char* NgineWindow::WindowClass::GetName()
	{
		return wndClassName;
	}

	HINSTANCE NgineWindow::WindowClass::GetInstance()
	{
		return wndClass.hInst;
	}

	NgineWindow::NgineWindow()
	{
		int width = FileUtils::GetIntegerFromConfig("Resource/ngine.ini", "General", "WindowWidth");
		int height = FileUtils::GetIntegerFromConfig("Resource/ngine.ini", "General", "WindowHeight");

		hWnd = CreateWindowEx(0, WindowClass::GetName(), "Ngine Xbox runtime", WS_OVERLAPPEDWINDOW, 0, 0, width, height, nullptr, nullptr, WindowClass::GetInstance(), this);

		if (hWnd == nullptr)
		{
			LOG_F(ERROR, "Failed to create Window!");
			throw Exception();
		}

		ShowWindow(hWnd, SW_SHOW);
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