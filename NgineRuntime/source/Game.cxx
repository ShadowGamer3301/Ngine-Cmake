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

	}
}

void Game::ManageEvents()
{
}

Ngine::Application* Ngine::GenerateNewApplicationInterface()
{
	return new Game();
}