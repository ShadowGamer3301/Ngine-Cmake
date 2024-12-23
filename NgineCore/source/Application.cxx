#include "Application.h"
#include "Window.h"

namespace Ngine
{
	Application::Application()
	{
		pWindow = new NgineWindow();
		pGfxCore = new GraphicsCore(pWindow);
	}

	Application::~Application()
	{
		if (pGfxCore) delete pGfxCore;
		if (pWindow) delete pWindow;
	}
}