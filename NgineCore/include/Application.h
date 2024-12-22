#pragma once
#include "Core.hxx"
#include "Window.h"
#include "GraphicsCore.h"

namespace Ngine
{
#if defined(TARGET_PLATFORM_WINDOWS) || defined(TARGET_PLATFORM_XBOX)
	class NGAPI Application;
#endif

	class Application
	{
	public:
		Application();
		virtual ~Application();

		virtual void Run() = 0;
		virtual void ManageEvents() = 0;

	protected:
		Window* pWindow;
		GraphicsCore* pGfxCore;
	};

	Application* GenerateNewApplicationInterface(); //Needs to be defined in client/runtime project
}