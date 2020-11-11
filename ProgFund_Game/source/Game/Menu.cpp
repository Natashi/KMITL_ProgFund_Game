#include "pch.h"
#include "Menu.hpp"
#include "StageMain.hpp"

//*******************************************************************
//Menu_Scene
//*******************************************************************
Menu_SplashScene::Menu_SplashScene(SceneManager* manager) : Scene(manager) {
	GET_INSTANCE(ResourceManager, resourceManager);

	//System
	resourceManager->LoadResource<TextureResource>("resource/img/system/ascii.png", "img/system/ascii.png");
	resourceManager->LoadResource<TextureResource>("resource/img/system/ascii_960.png", "img/system/ascii_960.png");

	//Splash
	resourceManager->LoadResource<TextureResource>("resource/img/menu/splash_2.png", "img/menu/splash_2.png");

	auto taskSplash = std::make_shared<Menu_SplashTask>(this);
	this->AddTask(taskSplash);

	bAutoDelete_ = true;
}
Menu_SplashScene::~Menu_SplashScene() {
	GET_INSTANCE(ResourceManager, resourceManager);

	resourceManager->RemoveResource("img/menu/splash_2.png");

	{
		shared_ptr<Menu_MainScene> nextScene(new Menu_MainScene(manager_));
		nextScene->SetType(Scene::Type::Menu);
		manager_->AddScene(nextScene, Scene::Type::Menu, true);
	}
}

//*******************************************************************
//Menu_SplashTask
//*******************************************************************
namespace MenuParams {
	static constexpr size_t SPLASH_TRANSITION = 60;
	static constexpr size_t SPLASH_WAITSPLASH = 120;
	static constexpr size_t SPLASH_TRANSWAIT = SPLASH_TRANSITION + SPLASH_WAITSPLASH;
	static constexpr size_t SPLASH_SINGLESEQ = SPLASH_TRANSWAIT + SPLASH_TRANSITION;
};

CONSTRUCT_TASK(Menu_SplashTask) {
	frameEnd_ = MenuParams::SPLASH_SINGLESEQ + 90;
	frameOff_ = 20;

	objSplash_.SetPosition(320, 240, 1);
}
Menu_SplashTask::~Menu_SplashTask() {
}
void Menu_SplashTask::Render(byte layer) {
	if (layer != 0) return;
	objSplash_.Render();
}
void Menu_SplashTask::Update() {
	GET_INSTANCE(InputManager, inputManager);
	GET_INSTANCE(ResourceManager, resourceManager);

	if (frameOff_ > 0) {
		--frameOff_;
		return;
	}

	if (frame_ == 0) {
		auto texture = resourceManager->GetResourceAs<TextureResource>("img/menu/splash_2.png");
		objSplash_.SetTexture(texture);
		objSplash_.SetSourceRect(DxRectangle(0, 0, 350, 256));
		objSplash_.SetDestCenter();
		objSplash_.UpdateVertexBuffer();
	}

	if (frame_ < MenuParams::SPLASH_SINGLESEQ) {
		bool isZPressed = inputManager->GetKeyState(VirtualKey::Ok) != KeyState::Free;
		if (isZPressed) {   //Skipping
			if (frame_ >= MenuParams::SPLASH_TRANSITION
				&& frame_ < MenuParams::SPLASH_TRANSWAIT)
				frame_ = MenuParams::SPLASH_TRANSWAIT;
		}

		if (frame_ < MenuParams::SPLASH_TRANSITION) {
			float tmp = frame_ / (float)(MenuParams::SPLASH_TRANSITION - 1);
			float scale = Math::Lerp::Decelerate(0.85f, 1.0f, tmp);
			objSplash_.SetScale(scale, scale, 1.0f);
			objSplash_.SetAlpha(Math::Lerp::Smooth(0, 255, tmp));
		}
		else if (frame_ >= MenuParams::SPLASH_TRANSWAIT) {
			float tmp = (frame_ - MenuParams::SPLASH_TRANSWAIT) / (float)(MenuParams::SPLASH_TRANSITION - 1);
			float scale = Math::Lerp::Accelerate(1.0f, 1.15f, tmp);
			objSplash_.SetScale(scale, scale, 1.0f);
			objSplash_.SetAlpha(Math::Lerp::Smooth(255, 0, tmp));
		}
	}

	++frame_;
}

//*******************************************************************
//MenuParams
//*******************************************************************
namespace MenuParams {
	static constexpr size_t MAIN_ITEMS = 3;
	static constexpr size_t MAIN_INDEX_START = 0;
	static constexpr size_t MAIN_INDEX_OPTIONS = 1;
	static constexpr size_t MAIN_INDEX_QUIT = 2;
};

//*******************************************************************
//Menu_Main_Items
//*******************************************************************
class Menu_Child_ParentMenu_Item : public TaskBase {
public:
	Menu_Child_ParentMenu_Item(Menu_MainScene* parent, CD3DXVECTOR2 pos, size_t index, size_t imgId) : TaskBase(parent) {
		index_ = index;
		pSelectIndex_ = nullptr;

		GET_INSTANCE(ResourceManager, resourceManager);
		auto textureItem = resourceManager->GetResourceAs<TextureResource>("img/menu/title_item00.png");

		itemObj_.SetTexture(textureItem);
		itemObjBack_.SetTexture(textureItem);
		{
			auto rect = DxRectangle<int>::SetFromIndex(256, 64, imgId, 2, 0, -4);
			itemObj_.SetSourceRect(rect);
			itemObj_.SetDestRect(DxRectangle(0, -32, 256, 32));

			rect = DxRectangle<int>::SetFromIndex(256, 64, imgId + 1, 2, 0, -4);
			itemObjBack_.SetSourceRect(rect);
			itemObjBack_.SetDestRect(DxRectangle(0, -32, 256, 32));

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

	virtual void Render() {
		itemObjBack_.Render();
		if (selectScale_ > 0.0f)
			itemObj_.Render();
	}
	virtual void Update() {
		if (pSelectIndex_ && (*pSelectIndex_ == index_))
			IncUntil(selectScale_, 1 / 8.0f, 1.0f);
		else
			DecUntil(selectScale_, 1 / 12.0f, 0.0f);

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
public:
	Sprite2D itemObj_;
	Sprite2D itemObjBack_;
	size_t index_;
	int* pSelectIndex_;

	float selectScale_;
};

//*******************************************************************
//Menu_Child_ParentMenu
//*******************************************************************
Menu_Child_ParentMenu::Menu_Child_ParentMenu(Menu_MainScene* parent) : TaskBase(parent) {
	selectIndex_ = 0;
	{
		listMenuObj_.resize(MenuParams::MAIN_ITEMS);
		size_t imgIds[] = { 0, 6, 8 };
		for (int i = 0; i < MenuParams::MAIN_ITEMS; ++i) {
			Menu_Child_ParentMenu_Item* item = new Menu_Child_ParentMenu_Item(parent,
				D3DXVECTOR2(48, 286 + i * 64), i, imgIds[i] * 2);
			item->pSelectIndex_ = &selectIndex_;
			listMenuObj_[i] = item;
		}
	}
}
Menu_Child_ParentMenu::~Menu_Child_ParentMenu() {
	for (auto& ptr : listMenuObj_)
		ptr_delete(ptr);
}

void Menu_Child_ParentMenu::Render(byte layer) {
	if (layer != 0) return;
	for (auto& ptr : listMenuObj_)
		ptr->Render();
}
void Menu_Child_ParentMenu::Update() {
	GET_INSTANCE(InputManager, inputManager);
	if (flgGetInput_ && frame_ > 20) {
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
			{
				auto taskMenu = std::make_shared<Menu_Child_RankMenu>((Menu_MainScene*)parent_);
				parent_->AddTask(taskMenu);
				break;
			}
			case MenuParams::MAIN_INDEX_OPTIONS:
			{
				break;
			}
			case MenuParams::MAIN_INDEX_QUIT:
			{
				break;
			}
			}

			frameEnd_ = 0;
		}
	}
	else {		//Wait until the user releases the Z key from the previous menu
		flgGetInput_ = inputManager->GetKeyState(VirtualKey::Ok) == KeyState::Free;
	}

	for (auto& ptr : listMenuObj_)
		ptr->Update();
	++frame_;
}

//*******************************************************************
//Menu_Child_RankMenu
//*******************************************************************
class Menu_Child_RankMenu_Item : public TaskBase {
public:
	Menu_Child_RankMenu_Item(Menu_MainScene* parent, CD3DXVECTOR2 pos, size_t index, size_t imgId) : TaskBase(parent) {
		index_ = index;
		pSelectIndex_ = nullptr;

		GET_INSTANCE(ResourceManager, resourceManager);
		auto textureItem = resourceManager->GetResourceAs<TextureResource>("img/menu/title_rank.png");

		itemObj_.SetTexture(textureItem);
		{
			auto rect = DxRectangle<int>::SetFromIndex(256, 128, imgId, 2);
			itemObj_.SetSourceRect(rect);
			itemObj_.SetDestCenter();

			itemObj_.UpdateVertexBuffer();
		}

		itemObj_.SetPosition(pos);
		itemObj_.SetAlpha(0);

		selectScale_ = 0.0f;
	}

	virtual void Render() {
		itemObj_.Render();
	}
	virtual void Update() {
		if (pSelectIndex_ && (*pSelectIndex_ == index_))
			IncUntil(selectScale_, 1 / 8.0f, 1.0f);
		else
			DecUntil(selectScale_, 1 / 12.0f, 0.0f);

		itemObj_.SetAlpha(Math::Lerp::Linear(48, 255, selectScale_));

		++frame_;
	}
public:
	Sprite2D itemObj_;
	size_t index_;
	int* pSelectIndex_;

	float selectScale_;
};

Menu_Child_RankMenu::Menu_Child_RankMenu(Menu_MainScene* parent) : TaskBase(parent) {
	GET_INSTANCE(ResourceManager, resourceManager);
	resourceManager->LoadResource<TextureResource>("resource/img/menu/title_rank.png", "img/menu/title_rank.png");

	{
		listRankObj_.resize(4);
		float posX[] = { -150, 150 };
		float posY[] = { -80, 80 };
		for (int i = 0; i < 4; ++i) {
			Menu_Child_RankMenu_Item* item = new Menu_Child_RankMenu_Item(parent,
				D3DXVECTOR2(320 + posX[i & 1], 240 + posY[i / 2]), i, i * 2);
			item->pSelectIndex_ = &selectIndex_;
			listRankObj_[i] = item;
		}
	}
	
	GET_INSTANCE(CommonDataManager, dataManager);
	if (ManagedValue* memory = dataManager->GetValue("GameRank")) {
		selectIndex_ = memory->GetAs<int>();
	}
	else {
		selectIndex_ = 1;
	}
}
Menu_Child_RankMenu::~Menu_Child_RankMenu() {
	for (auto& ptr : listRankObj_)
		ptr_delete(ptr);
}

void Menu_Child_RankMenu::Render(byte layer) {
	if (layer != 0) return;
	for (auto& ptr : listRankObj_)
		ptr->Render();
}
void Menu_Child_RankMenu::Update() {
	GET_INSTANCE(InputManager, inputManager);
	if (flgGetInput_ && frame_ > 20 && frameEnd_ == UINT_MAX) {
		if (inputManager->GetKeyState(VirtualKey::Down) == KeyState::Push) {
			selectIndex_ += 2;
		}
		else if (inputManager->GetKeyState(VirtualKey::Up) == KeyState::Push) {
			selectIndex_ -= 2;
		}
		else if (inputManager->GetKeyState(VirtualKey::Left) == KeyState::Push) {
			selectIndex_ = selectIndex_ < 2 ? (1 - selectIndex_) : (5 - selectIndex_);
		}
		else if (inputManager->GetKeyState(VirtualKey::Right) == KeyState::Push) {
			selectIndex_ = selectIndex_ < 2 ? (1 - selectIndex_) : (5 - selectIndex_);
		}
		else if (inputManager->GetKeyState(VirtualKey::Ok) == KeyState::Push) {
			GET_INSTANCE(CommonDataManager, dataManager);
			dataManager->SetValue("GameRank", selectIndex_);

			frameEnd_ = frame_ + 40;

			{
				auto rearScene = parent_->GetParentManager()->GetRearScene().get();
				auto taskFade = shared_ptr<UtilTask_ColorFade>(
					new UtilTask_ColorFade(rearScene, 30, 30, 70, D3DCOLOR_XRGB(0, 0, 0)));
				rearScene->AddTask(taskFade);
			}
			{
				auto primaryScene = parent_->GetParentManager()->GetPrimaryScene().get();
				auto taskStageLoad = shared_ptr<Stage_SceneLoader>(new Stage_SceneLoader(primaryScene));
				primaryScene->AddTask(taskStageLoad);
			}
		}
		else if (inputManager->GetKeyState(VirtualKey::Cancel) == KeyState::Push) {
			GET_INSTANCE(CommonDataManager, dataManager);
			dataManager->SetValue("GameRank", selectIndex_);

			frameEnd_ = 0;

			auto taskMenu = std::make_shared<Menu_Child_ParentMenu>((Menu_MainScene*)parent_);
			parent_->AddTask(taskMenu);
		}

		if (selectIndex_ < 0)
			selectIndex_ += 4;
		else if (selectIndex_ >= 4)
			selectIndex_ -= 4;
	}
	else {		//Wait until the user releases the Z key from the previous menu
		flgGetInput_ = inputManager->GetKeyState(VirtualKey::Ok) == KeyState::Free;
	}

	for (auto& ptr : listRankObj_)
		ptr->Update();
	++frame_;
}

//*******************************************************************
//Menu_MainTask
//*******************************************************************
Menu_MainScene::Menu_MainScene(SceneManager* manager) : Scene(manager) {
	GET_INSTANCE(ResourceManager, resourceManager);
	resourceManager->LoadResource<TextureResource>(
		"resource/img/menu/title_item00.png", "img/menu/title_item00.png");

	{
		musicBackground_ = resourceManager->LoadResource<SoundResource>(
			"resource/sound/music/title.ogg", "sound/music/title.ogg");
		musicBackground_->GetData()->Play();
		musicBackground_->GetData()->SetVolumeRate(70);
	}

	{
		auto textureBackground = resourceManager->LoadResource<TextureResource>(
			"resource/img/menu/title_back.png", "img/menu/title_back.png");

		objBackground_.SetTexture(textureBackground);

		objBackground_.SetSourceRect(DxRectangle(0, 0, 640, 480));
		objBackground_.SetDestRect(DxRectangle(0, 0, 640, 480));
		objBackground_.UpdateVertexBuffer();

		objBackground_.SetScale(1.5, 1.5, 1);
		objBackground_.SetAlpha(0);
	}
	{
		auto taskMenu = std::make_shared<Menu_Child_ParentMenu>(this);
		this->AddTask(taskMenu);
	}

	bAutoDelete_ = true;
}
Menu_MainScene::~Menu_MainScene() {
	auto primaryScene = manager_->GetPrimaryScene();
	primaryScene->AddTask(std::shared_ptr<UtilTask_FadeBGM>(
		new UtilTask_FadeBGM(primaryScene.get(), musicBackground_, 60, 
			musicBackground_->GetData()->GetVolumeRate())));
}
void Menu_MainScene::Render() {
	{
		float scrollF = (float)(frame_ % 1200) / 1200;
		objBackground_.SetScroll(scrollF, frame_ / 1200.0);
		objBackground_.Render();
		objBackground_.SetScroll(-scrollF + 0.34f, frame_ / 2000.0 + 0.36f);
		objBackground_.Render();
	}
	Scene::Render();
}
void Menu_MainScene::Update() {
	{
		float mul = frame_ < 180 ? frame_ / 180.0f * 0.5f : 0.5f;
		float sina = (float)(frame_ % 340) / 340 * GM_PI_X2;
		objBackground_.SetAlpha((128 + 128 * sinf(sina)) * mul);
	}
	Scene::Update();
}