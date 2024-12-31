#include "Game.h"
#include "Event.h"
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

	mShader = pGfxCore->LoadShader("Resource/Shader/mvp_vert.spv", "Resource/Shader/mvp_frag.spv");

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
	mModel2 = pGfxCore->LoadIntermediateModel("cube.fbx");

	pObject = new Ngine::GameObject3D(mModel, mShader);
	pObject2 = new Ngine::GameObject3D(mModel2, mShader);
	pGfxCore->AddGameObjectToDrawList(pObject2);
	pGfxCore->AddGameObjectToDrawList(pObject);
	pGfxCore->Temp_SetCamera(glm::vec3(0,0,0));
	//pObject2->SetScale(glm::vec3(10.0f, 10.0f, 10.0f));

	mCamera.SetProjectionValues(60.0f, 1920/(float)1080, 0.01f, 1000.0f);
	//mCamera.SetPosition(glm::vec3(2.0f, 2.0f, 2.0f));
}

Game::~Game()
{
}

void Game::Run()
{
	while (pWindow->UpdateWindow())
	{
		ManageEvents();
		pGfxCore->SetCamera(mCamera);
		pGfxCore->DrawFrame(pWindow);
	}
}

void Game::ManageEvents()
{
	auto& eb = Ngine::EventHandler::ObtainEventBuffer();

	for (auto& element : eb)	
	{
		if(element->mType == Ngine::EventType::EventType_Null)
			continue;

		if(element->mType == Ngine::EventType::EventType_WindowResize)
			continue; //TODO : WINDOW RESIZING

		if(element->mType == Ngine::EventType::EventType_KeyAction)
		{
			Ngine::EventKeyAction* pCastedEvent = reinterpret_cast<Ngine::EventKeyAction*>(element);
			
			if(pCastedEvent->mPressed && pCastedEvent->mKey == GLFW_KEY_W)
				mCamera.AdjustPosition(glm::vec3(0,0,1));
			if(pCastedEvent->mPressed && pCastedEvent->mKey == GLFW_KEY_S)
				mCamera.AdjustPosition(glm::vec3(0,0,-1));
			if(pCastedEvent->mPressed && pCastedEvent->mKey == GLFW_KEY_A)
				mCamera.AdjustPosition(glm::vec3(-1,0,0));
			if(pCastedEvent->mPressed && pCastedEvent->mKey == GLFW_KEY_D)
				mCamera.AdjustPosition(glm::vec3(1,0,0));
			if(pCastedEvent->mPressed && pCastedEvent->mKey == GLFW_KEY_Q)
				mCamera.AdjustRotation(glm::vec3(0,-1,0));
			if(pCastedEvent->mPressed && pCastedEvent->mKey == GLFW_KEY_E)
				mCamera.AdjustRotation(glm::vec3(0,1,0));
		}
		if(element->mType == Ngine::EventType::EventType_CursorMove)
		{
			Ngine::EventCursorMove* pCastedEvent = reinterpret_cast<Ngine::EventCursorMove*>(element);
			LOG_F(INFO, "Cursor moved to %f x %f", pCastedEvent->pos_x, pCastedEvent->pos_y);
		}
	}

	Ngine::EventHandler::ClearEventBuffer();
}

Ngine::Application* Ngine::GenerateNewApplicationInterface()
{
	return new Game();
}