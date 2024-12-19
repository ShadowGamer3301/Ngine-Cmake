#pragma once
#include <Ngine.hxx>

class Game : public Ngine::Application
{
public:
	Game();
	~Game();

	void Run() override;
	void ManageEvents() override;
};