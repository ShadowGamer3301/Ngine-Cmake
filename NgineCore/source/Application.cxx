#include "Application.h"

namespace Ngine
{
	Application::Application()
	{
		pWindow = new Window();
	}

	Application::~Application()
	{
		if (pWindow) delete pWindow;
	}
}