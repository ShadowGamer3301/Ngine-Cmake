#include "Application.h"

namespace Ngine
{
	Application::Application()
	{
		pWindow = new Window();
		pGfxCore = new GraphicsCore(pWindow);
	}

	Application::~Application()
	{
		if (pGfxCore) delete pGfxCore;
		if (pWindow) delete pWindow;
	}
}