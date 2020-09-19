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
Menu_SplashScene::Menu_SplashScene(Scene* parent) : TaskBase(parent) {
    frameEnd_ = (40 + 40 + 120) * 2;

    objSplash_.SetPosition(320, 240, 1);
	objSplash_.SetSourceRectNormalized(GLRect(0, 0, 1, 1));
}
void Menu_SplashScene::Render() {
	objSplash_.Render();
}
void Menu_SplashScene::Update() {
    InputManager* inputManager = InputManager::GetBase();
    ResourceManager* resourceManager = ResourceManager::GetBase();

    int step = frame_ / 200;
    int _frame = frame_ - step * 200;
    sf::Vector2f texSize = step == 0 ? sf::Vector2f(440, 180) : sf::Vector2f(350, 260);

    bool isZPressed = inputManager->GetKeyState(VirtualKey::Ok) != KeyState::Free;
    if (isZPressed) {   //Skipping
        if (_frame >= 40 && _frame < 160) {
            frame_ = step * 200 + 160;
            _frame = 160;
        }
    }

    if (_frame < 40) {
        if (_frame == 0) {
            std::string path = step == 0 ? "img/menu/splash_1.png" : "img/menu/splash_2.png";
            auto texture = resourceManager->GetResourceAs<TextureResource>(path);
            objSplash_.SetTexture(texture);
			//objSplash_.SetDestCenter();
			objSplash_.SetDestRect(GLRect(-1, -1, 1, 1));
            //objSplash_.setPosition(320 - texSize.x * 0.5f, 240 - texSize.y * 0.5f);
        }
        int alpha = Math::Lerp::Smooth(0, 255, _frame / 39.0f);
        objSplash_.SetAlpha(alpha);
    }
    else if (_frame >= 120 + 40) {
        int alpha = Math::Lerp::Smooth(255, 0, (_frame - 120 - 40) / 39.0f);
        objSplash_.SetAlpha(alpha);
    }

    ++frame_;
}