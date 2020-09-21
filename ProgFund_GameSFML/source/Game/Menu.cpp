#include "pch.h"
#include "Menu.hpp"

//*******************************************************************
//Menu_TaskHost
//*******************************************************************
Menu_TaskHost::Menu_TaskHost(SceneManager* manager) : Scene(manager) {
	ResourceManager* resourceManager = ResourceManager::GetBase();

    auto imgSplash1 = std::make_shared<TextureResource>();
    imgSplash1->LoadFromFile(PathProperty::GetModuleDirectory() + "resource/img/menu/splash_1.png");
    resourceManager->AddResource(imgSplash1, "img/menu/splash_1.png");

    auto imgSplash2 = std::make_shared<TextureResource>();
    imgSplash2->LoadFromFile(PathProperty::GetModuleDirectory() + "resource/img/menu/splash_2.png");
    resourceManager->AddResource(imgSplash2, "img/menu/splash_2.png");

    auto taskSplash = std::make_shared<Menu_SplashScene>(this);
    this->AddTask(taskSplash);
}
Menu_TaskHost::~Menu_TaskHost() {
    ResourceManager* resourceManager = ResourceManager::GetBase();

    resourceManager->RemoveResource("img/menu/splash_1.png");
    resourceManager->RemoveResource("img/menu/splash_2.png");
}

//*******************************************************************
//Menu_SplashScene
//*******************************************************************
constexpr size_t TIMER_TRANSITION = 40;
constexpr size_t TIMER_WAITSPLASH = 180;
constexpr size_t TIMER_SINGLESEQ = TIMER_TRANSITION * 2U + TIMER_WAITSPLASH;
Menu_SplashScene::Menu_SplashScene(Scene* parent) : TaskBase(parent) {
    frameEnd_ = TIMER_SINGLESEQ * 2;

    objSplash_.SetPosition(320, 240, 1);
}
void Menu_SplashScene::Render() {
	objSplash_.Render();
}
void Menu_SplashScene::Update() {
	InputManager* inputManager = InputManager::GetBase();
	ResourceManager* resourceManager = ResourceManager::GetBase();

	int step = frame_ / TIMER_SINGLESEQ;
	int _frame = frame_ - step * TIMER_SINGLESEQ;

    bool isZPressed = inputManager->GetKeyState(VirtualKey::Ok) != KeyState::Free;
    if (isZPressed) {   //Skipping
        if (_frame >= TIMER_TRANSITION && _frame < TIMER_TRANSITION + TIMER_WAITSPLASH) {
            frame_ = step * TIMER_SINGLESEQ + (TIMER_TRANSITION + TIMER_WAITSPLASH);
            _frame = TIMER_TRANSITION + TIMER_WAITSPLASH;
        }
    }

    if (_frame < TIMER_TRANSITION) {
        if (_frame == 0) {
			D3DXVECTOR2 texSize = step == 0 ? D3DXVECTOR2(440, 180) : D3DXVECTOR2(350, 256);
            std::string path = step == 0 ? "img/menu/splash_1.png" : "img/menu/splash_2.png";
            auto texture = resourceManager->GetResourceAs<TextureResource>(path);
            objSplash_.SetTexture(texture);
			objSplash_.SetSourceRect(DxRect(0, 0, texSize.x, texSize.y));
			objSplash_.SetDestCenter();
			objSplash_.UpdateVertexBuffer();
        }
        byte alpha = Math::Lerp::Smooth(0, 255, _frame / (float)(TIMER_TRANSITION - 1));
        objSplash_.SetAlpha(alpha);
    }
    else if (_frame >= TIMER_TRANSITION + TIMER_WAITSPLASH) {
		byte alpha = Math::Lerp::Smooth(255, 0, (_frame - (TIMER_TRANSITION + TIMER_WAITSPLASH)) / (float)(TIMER_TRANSITION - 1));
        objSplash_.SetAlpha(alpha);
    }

    ++frame_;
}