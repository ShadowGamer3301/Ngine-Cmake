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
		std::cout << "If you read this you are running Xbox build!" << std::endl;
	}
}

void Game::ManageEvents()
{
}

Ngine::Application* Ngine::GenerateNewApplicationInterface()
{
	return new Game();
}