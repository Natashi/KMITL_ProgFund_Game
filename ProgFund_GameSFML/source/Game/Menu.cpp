#include "pch.h"
#include "Menu.hpp"

//*******************************************************************
//Menu_Scene
//*******************************************************************
Menu_Scene::Menu_Scene(SceneManager* manager) : Scene(manager) {
	ResourceManager* resourceManager = ResourceManager::GetBase();

	resourceManager->LoadResource<TextureResource>("resource/img/menu/splash_1.png", "img/menu/splash_1.png");
	resourceManager->LoadResource<TextureResource>("resource/img/menu/splash_2.png", "img/menu/splash_2.png");

    auto taskSplash = std::make_shared<Menu_SplashTask>(this);
    this->AddTask(taskSplash);

	bAutoDelete_ = true;
}
Menu_Scene::~Menu_Scene() {
    ResourceManager* resourceManager = ResourceManager::GetBase();

    resourceManager->RemoveResource("img/menu/splash_1.png");
    resourceManager->RemoveResource("img/menu/splash_2.png");
}

//*******************************************************************
//Menu_SplashTask
//*******************************************************************
namespace MenuParams {
	static constexpr size_t SPLASH_TRANSITION = 60;
	static constexpr size_t SPLASH_WAITSPLASH = 150;
	static constexpr size_t SPLASH_TRANSWAIT = SPLASH_TRANSITION + SPLASH_WAITSPLASH;
	static constexpr size_t SPLASH_SINGLESEQ = SPLASH_TRANSWAIT + SPLASH_TRANSITION;
};
Menu_SplashTask::Menu_SplashTask(Scene* parent) : TaskBase(parent) {
    frameEnd_ = MenuParams::SPLASH_SINGLESEQ * 2 + 90;
	frameOff_ = 20;

    objSplash_.SetPosition(320, 240, 1);
}
Menu_SplashTask::~Menu_SplashTask() {
	auto taskMenu = std::make_shared<Menu_MainTask>(parent_);
	parent_->AddTask(taskMenu);
}
void Menu_SplashTask::Render() {
	objSplash_.Render();
}
void Menu_SplashTask::Update() {
	InputManager* inputManager = InputManager::GetBase();
	ResourceManager* resourceManager = ResourceManager::GetBase();

	if (frameOff_ > 0) {
		--frameOff_;
		return;
	}

	if (frame_ < MenuParams::SPLASH_SINGLESEQ * 2) {
		int step = frame_ / MenuParams::SPLASH_SINGLESEQ;
		int _frame = frame_ - step * MenuParams::SPLASH_SINGLESEQ;

		bool isZPressed = inputManager->GetKeyState(VirtualKey::Ok) != KeyState::Free;
		if (isZPressed) {   //Skipping
			if (_frame >= MenuParams::SPLASH_TRANSITION
				&& _frame < MenuParams::SPLASH_TRANSWAIT)
			{
				frame_ = step * MenuParams::SPLASH_SINGLESEQ + (MenuParams::SPLASH_TRANSWAIT);
				_frame = MenuParams::SPLASH_TRANSWAIT;
			}
		}

		if (_frame < MenuParams::SPLASH_TRANSITION) {
			if (_frame == 0) {
				D3DXVECTOR2 texSize = step == 0 ? D3DXVECTOR2(440, 180) : D3DXVECTOR2(350, 256);
				std::string path = step == 0 ? "img/menu/splash_1.png" : "img/menu/splash_2.png";
				auto texture = resourceManager->GetResourceAs<TextureResource>(path);
				objSplash_.SetTexture(texture);
				objSplash_.SetSourceRect(DxRect(0, 0, texSize.x, texSize.y));
				objSplash_.SetDestCenter();
				objSplash_.UpdateVertexBuffer();
			}
			float tmp = _frame / (float)(MenuParams::SPLASH_TRANSITION - 1);
			float scale = Math::Lerp::Decelerate(0.85f, 1.0f, tmp);
			objSplash_.SetScale(scale, scale, 1.0f);
			objSplash_.SetAlpha(Math::Lerp::Smooth(0, 255, tmp));
		}
		else if (_frame >= MenuParams::SPLASH_TRANSWAIT) {
			float tmp = (_frame - MenuParams::SPLASH_TRANSWAIT) / (float)(MenuParams::SPLASH_TRANSITION - 1);
			float scale = Math::Lerp::Accelerate(1.0f, 1.15f, tmp);
			objSplash_.SetScale(scale, scale, 1.0f);
			objSplash_.SetAlpha(Math::Lerp::Smooth(255, 0, tmp));
		}
	}

    ++frame_;
}

//*******************************************************************
//Menu_MainTask
//*******************************************************************
namespace MenuParams {
	static constexpr size_t MAIN_ITEMS = 3;
	static constexpr size_t MAIN_INDEX_START	= 0;
	static constexpr size_t MAIN_INDEX_OPTIONS	= 1;
	static constexpr size_t MAIN_INDEX_QUIT		= 2;
};
Menu_MainTask::Menu_MainTask(Scene* parent) : TaskBase(parent) {
	ResourceManager* resourceManager = ResourceManager::GetBase();
	resourceManager->LoadResource<TextureResource>(
		"resource/img/menu/title_item00.png", "img/menu/title_item00.png");

	{
		auto textureBackground = resourceManager->LoadResource<TextureResource>(
			"resource/img/menu/title_back.png", "img/menu/title_back.png");

		objBackground_.SetTexture(textureBackground);

		objBackground_.SetSourceRect(DxRect(0, 0, 640, 480));
		objBackground_.SetDestRect(DxRect(0, 0, 640, 480));
		objBackground_.UpdateVertexBuffer();

		objBackground_.SetScale(1.5, 1.5, 1);
		objBackground_.SetAlpha(0);
	}

	selectIndex_ = 0;
	{
		listMenuObj_.resize(MenuParams::MAIN_ITEMS);
		size_t imgIds[] = { 0, 6, 8 };
		for (int i = 0; i < MenuParams::MAIN_ITEMS; ++i) {
			Menu_Main_Items* item = new Menu_Main_Items(parent,
				D3DXVECTOR2(48, 286 + i * 64), i, imgIds[i] * 2);
			item->SetSelectIndexPointer(&selectIndex_);
			listMenuObj_[i] = item;
		}
	}
}
Menu_MainTask::~Menu_MainTask() {
	for (auto& ptr : listMenuObj_)
		ptr_delete(ptr);
}
void Menu_MainTask::Render() {
	{
		float scrollF = (float)(frame_ % 1200) / 1200;
		objBackground_.SetScroll(scrollF, frame_ / 1200.0);
		objBackground_.Render();
		objBackground_.SetScroll(-scrollF + 0.34f, frame_ / 2000.0 + 0.36f);
		objBackground_.Render();
	}
	for (auto& ptr : listMenuObj_)
		ptr->Render();
}
void Menu_MainTask::Update() {
	InputManager* inputManager = InputManager::GetBase();

	{
		float mul = frame_ < 180 ? frame_ / 180.0f * 0.5f : 0.5f;
		float sina = (float)(frame_ % 340) / 340 * GM_PI_X2;
		objBackground_.SetAlpha((128 + 128 * sinf(sina)) * mul);
	}

	if (inputManager->GetKeyState(VirtualKey::Down) == KeyState::Push) {
		++selectIndex_;
		if (selectIndex_ >= MenuParams::MAIN_ITEMS)
			selectIndex_ = 0;
	}
	else if (inputManager->GetKeyState(VirtualKey::Up) == KeyState::Push) {
		--selectIndex_;
		if (selectIndex_ < 0)
			selectIndex_ = MenuParams::MAIN_ITEMS - 1;
	}
	else if (inputManager->GetKeyState(VirtualKey::Ok) == KeyState::Push) {
		switch (selectIndex_) {
		case MenuParams::MAIN_INDEX_START:
		case MenuParams::MAIN_INDEX_OPTIONS:
			break;
		case MenuParams::MAIN_INDEX_QUIT:
		{
			frameEnd_ = 0;
			break;
		}
		}
	}

	for (auto& ptr : listMenuObj_)
		ptr->Update();

	++frame_;
}

//*******************************************************************
//Menu_Main_Items
//*******************************************************************
Menu_Main_Items::Menu_Main_Items(Scene* parent, CD3DXVECTOR2 pos, size_t index, size_t imgId) : TaskBase(parent) {
	index_ = index;
	pSelectIndex_ = nullptr;

	ResourceManager* resourceManager = ResourceManager::GetBase();
	auto textureItem = resourceManager->GetResourceAs<TextureResource>("img/menu/title_item00.png");

	itemObj_.SetTexture(textureItem);
	itemObjBack_.SetTexture(textureItem);
	{
		DxRect rect = DxRect::SetFromIndex(256, 64, imgId, 2, 0, -4);
		itemObj_.SetSourceRect(rect);
		itemObj_.SetDestRect(DxRect(0, -32, 256, 32));

		rect = DxRect::SetFromIndex(256, 64, imgId + 1, 2, 0, -4);
		itemObjBack_.SetSourceRect(rect);
		itemObjBack_.SetDestRect(DxRect(0, -32, 256, 32));

		itemObj_.UpdateVertexBuffer();
		itemObjBack_.UpdateVertexBuffer();
	}

	itemObjBack_.SetScale(0.75f, 0.75f, 1.0f);
	itemObj_.SetPosition(pos);
	itemObjBack_.SetPosition(pos);

	itemObjBack_.SetAlpha(0);
	itemObj_.SetAlpha(0);

	selectScale_ = 0.0f;
}
void Menu_Main_Items::Render() {
	itemObjBack_.Render();
	if (selectScale_ > 0.0f)
		itemObj_.Render();
}
void Menu_Main_Items::Update() {
	if (pSelectIndex_  && (*pSelectIndex_ == index_))
		IncUntil(selectScale_, 1 / 12.0f, 1.0f);
	else
		DecUntil(selectScale_, 1 / 16.0f, 0.0f);

	{
		float mul = frame_ < 30 ? frame_ / 30.0f : 1.0f;
		itemObjBack_.SetAlpha(255 * mul);
	}

	float alphaSin = 1.0f + sinf(Math::DegreeToRadian(frame_) * 3.0f) * 0.15f;
	itemObj_.SetAlpha(Math::Lerp::Linear(0, 255, selectScale_) * alphaSin);
	itemObj_.SetScaleX(Math::Lerp::Linear(0.0f, 1.0f, selectScale_) * 0.75f);
	itemObj_.SetScaleY(Math::Lerp::Linear(1.5f, 1.0f, selectScale_) * 0.75f);

	++frame_;
}