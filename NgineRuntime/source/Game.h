#pragma once
#include <Ngine.hxx>

class Game : public Ngine::Application
{
public:
	Game();
	~Game();

	void Run() override;
	void ManageEvents() override;

private:
	uint32_t mShader = 0;
	uint32_t mModel = 0;
	uint32_t mModel2 = 0;

	Ngine::GameObject3D* pObject;
	Ngine::GameObject3D* pObject2;
	Ngine::Camera mCamera;
};