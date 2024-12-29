#include "Game.h"
#include "FileUtils.h"

Game::Game()
{
	Ngine::BinaryConfig test_conf = Ngine::FileUtils::LoadBinaryConfig("Resource/ngine.ncf");

	LOG_F(WARNING, "Config data: WindowFullscreen: %s", test_conf.WindowFullscreen?"true":"false");
	LOG_F(WARNING, "Config data: WindowWidth: %d", test_conf.WindowWidth);
	LOG_F(WARNING, "Config data: WindowHeight: %d", test_conf.WindowHeight);
	LOG_F(WARNING, "Config data: AutoPickDevice: %s", test_conf.AutoPickDevice?"true":"false");
	LOG_F(WARNING, "Config data: ManualDeviceIndex: %d", test_conf.ManualDeviceIndex);
	LOG_F(WARNING, "Config data: EnableGfxDebugMode: %s", test_conf.EnableGfxDebugMode?"true":"false");
	LOG_F(WARNING, "Config data: WindowResize: %s", test_conf.WindowResize?"true":"false");
}

Game::~Game()
{
}

void Game::Run()
{
	while (pWindow->UpdateWindow())
	{
		pGfxCore->DrawFrame(pWindow);
	}
}

void Game::ManageEvents()
{
}

Ngine::Application* Ngine::GenerateNewApplicationInterface()
{
	return new Game();
}