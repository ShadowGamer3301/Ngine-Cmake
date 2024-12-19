#pragma once
#include "Application.h"
#include "Exception.h"

extern Ngine::Application* Ngine::GenerateNewApplicationInterface();

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