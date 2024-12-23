#include "Game.h"

Game::Game()
{

}

Game::~Game()
{
}

void Game::Run()
{
	while (pWindow->UpdateWindow())
	{
		pGfxCore->RenderFrame();
	}
}

void Game::ManageEvents()
{
}

Ngine::Application* Ngine::GenerateNewApplicationInterface()
{
	return new Game();
}