#pragma once
#include "Application.h"
#include "Exception.h"

extern Ngine::Application* Ngine::GenerateNewApplicationInterface();

#if defined(TARGET_PLATFORM_WINDOWS) || defined(TARGET_PLATFORM_LINUX)

int main(void) try
{
	auto app = Ngine::GenerateNewApplicationInterface();
	app->Run();
	delete app;

	return 0;
}
catch (Ngine::Exception& e)
{
	printf("%s", e.what());
	return 1;
}

#elif defined(TARGET_PLATFORM_XBOX)

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInts, LPCSTR nCmdLine, int nCmdShow)

#endif