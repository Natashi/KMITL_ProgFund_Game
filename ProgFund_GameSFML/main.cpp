#include "pch.h"

#include "source/Engine/ResourceManager.hpp"
#include "source/Engine/Input.hpp"
#include "source/Engine/Window.hpp"
#include "source/Engine/Scene.hpp"

#include "source/Game/Menu.hpp"

DWORD DxGetTime();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
	HWND hWnd = nullptr;
	try {
		printf("Initializing application...\n");

		WindowMain* winMain = new WindowMain();
		winMain->Initialize(hInstance);
		hWnd = winMain->GetHandle();

		ResourceManager* resourceManager = new ResourceManager();
		resourceManager->Initialize();
		/*
		{
			std::string path = PathProperty::GetModuleDirectory() + "resource/font/GN-Kin-iro_SansSerif.ttf";
			auto fntKinIro = std::make_shared<FontResource>();
			fntKinIro->LoadFromFile(path);
			resourceManager->AddResource(fntKinIro, "font/GN-Kin-iro_SansSerif.ttf");

			fpsCounter.setFont(*fntKinIro->GetData());
		}
		*/

		InputManager* inputManager = new InputManager();
		inputManager->Initialize();

		SceneManager* sceneManager = new SceneManager();
		sceneManager->Initialize();

		printf("Initialized application.\n");

		{
			Menu_TaskHost* menuScene = new Menu_TaskHost(sceneManager);
			menuScene->SetType(Scene::Type::Menu);
			sceneManager->AddScene(menuScene, (size_t)Scene::Type::Menu);
			printf("Initialized game.\n");
		}

		{
			//Refresh rate -> 60fps
			double t_target_ms = 1000.0 / 60;

			DWORD current_time = DxGetTime();
			DWORD previous_time = current_time;

			DWORD accum_fps = 0;
			DWORD accum_update = 0;

			std::list<DWORD> listDelta;

			MSG msg = { 0 };
			while (msg.message != WM_QUIT) {
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				else {
					current_time = DxGetTime();
					DWORD delta_time = current_time - previous_time;

					accum_fps += delta_time;
					accum_update += delta_time;

					if (accum_update > t_target_ms) {
						listDelta.push_back(accum_update);

						//Engine update
						inputManager->Update();
						sceneManager->Update();

						//Engine render
						winMain->BeginScene();
						sceneManager->Render();
						/*
						{
							//Render the FPS counter
							window->pushGLStates();
							winMain->SetBlendMode(BlendMode::Alpha);
							window->draw(fpsCounter);
							window->popGLStates();
						}
						*/
						winMain->EndScene();

						accum_update = 0;
						previous_time = DxGetTime();
					}

					//2 fps updates per second
					if (accum_fps > 500) {
						DWORD sumMs = 0;
						for (DWORD& iMs : listDelta)
							sumMs += iMs;
						listDelta.clear();

						double fps = (t_target_ms * 1000) / (double)sumMs;
						winMain->SetFPS(fps);

						//fpsCounter.setString(StringFormat("%.2f fps", fps));
						//printf("%.2f\n", fps);

						accum_fps = 0;
					}

					Sleep(1);
				}
			}
		}

		printf("Finalizing application...\n");

		ptr_release(resourceManager);
		ptr_release(sceneManager);
		ptr_release(inputManager);
		ptr_release(winMain);

		printf("Finalized application.\n");
		return 0;
	}
	catch (std::exception& e) {
		MessageBoxA(hWnd, e.what(), "Unexpected Error", MB_ICONERROR | MB_APPLMODAL | MB_OK);
	}
	catch (EngineError& e) {
		MessageBoxA(hWnd, e.what(), "Engine Error", MB_ICONERROR | MB_APPLMODAL | MB_OK);
	}

	return 0;
}

DWORD DxGetTime() {
	LARGE_INTEGER nFreq;
	LARGE_INTEGER nCounter;
	QueryPerformanceFrequency(&nFreq);
	QueryPerformanceCounter(&nCounter);
	return (DWORD)(nCounter.QuadPart * 1000 / nFreq.QuadPart);
}