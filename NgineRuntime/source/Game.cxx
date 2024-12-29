#include "Game.h"
#include "FileUtils.h"
#include "GameObject.h"

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

	mShader = pGfxCore->LoadShader("Resource/Shader/vertex_mvp.spv", "Resource/Shader/fragment_mvp.spv");

	std::vector<Ngine::Vertex> vertices = {
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}
	};

	std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
	};

	mModel = pGfxCore->CreateModelFromVertexList(vertices, indices);

	pObject = new Ngine::GameObject3D(mModel, mShader);
	pGfxCore->AddGameObjectToDrawList(pObject);
	pGfxCore->Temp_SetCamera(glm::vec3(0,0,0));
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