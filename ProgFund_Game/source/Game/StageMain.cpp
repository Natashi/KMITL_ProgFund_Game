#include "pch.h"

#include "StageMain.hpp"

#include "Player.hpp"

//*******************************************************************
//Stage_SceneLoader
//*******************************************************************
CONSTRUCT_TASK(Stage_SceneLoader) {
	frameEnd_ = 30;

	SceneManager* manager = parent_->GetParentManager();

	{
		GET_INSTANCE(ResourceManager, resourceManager);

		//Stage UI
		resourceManager->LoadResource<TextureResource>(
			"resource/img/stage/stg_frame.png", "img/stage/stg_frame.png");

		//Stage main
		resourceManager->LoadResource<TextureResource>(
			"resource/img/stage/eff_aura.png", "img/stage/eff_aura.png");
		resourceManager->LoadResource<TextureResource>(
			"resource/img/stage/eff_magicsquare.png", "img/stage/eff_magicsquare.png");

		resourceManager->LoadResource<TextureResource>(
			"resource/img/stage/dot_shinki.png", "img/stage/dot_shinki.png");

		//Player
		resourceManager->LoadResource<TextureResource>(
			"resource/img/player/eff_base.png", "img/player/eff_base.png");
		resourceManager->LoadResource<TextureResource>(
			"resource/img/player/eff_dead.png", "img/player/eff_dead.png");
		resourceManager->LoadResource<TextureResource>(
			"resource/img/player/eff_sloweffect.png", "img/player/eff_sloweffect.png");
		resourceManager->LoadResource<TextureResource>(
			"resource/img/player/pl00.png", "img/player/pl00.png");

	}
}
Stage_SceneLoader::~Stage_SceneLoader() {
	SceneManager* manager = parent_->GetParentManager();
	{
		shared_ptr<Stage_MainScene> nextScene(new Stage_MainScene(manager));
		nextScene->SetType(Scene::Type::Stage);
		manager->AddScene(nextScene, Scene::Type::Stage, true);
	}
	{
		shared_ptr<Stage_MainSceneUI> nextScene(new Stage_MainSceneUI(manager));
		nextScene->SetType(Scene::Type::StageUI);
		manager->AddScene(nextScene, Scene::Type::StageUI, true);
	}
	manager->RemoveScene(Scene::Type::Menu);
}
void Stage_SceneLoader::Update() {
	++frame_;
}

//*******************************************************************
//Stage_MainSceneUI
//*******************************************************************
Stage_MainSceneUI::Stage_MainSceneUI(SceneManager* manager) : Scene(manager) {
	GET_INSTANCE(ResourceManager, resourceManager);

	{
		auto textureFrame = resourceManager->GetResourceAs<TextureResource>("img/stage/stg_frame.png");
		objFrame_.SetTexture(textureFrame);
		objFrame_.SetSourceRect(DxRect(0, 0, 640, 480));
		objFrame_.SetDestRect(DxRect(0, 0, 640, 480));
		objFrame_.UpdateVertexBuffer();
	}
	objFrame_.SetBlendType(BlendMode::Invert);

	bAutoDelete_ = false;
}
Stage_MainSceneUI::~Stage_MainSceneUI() {
}

void Stage_MainSceneUI::Render() {
	objFrame_.Render();

	Scene::Render();
}
void Stage_MainSceneUI::Update() {
	Scene::Update();
}

//*******************************************************************
//Stage_MainScene
//*******************************************************************
Stage_MainScene::Stage_MainScene(SceneManager* manager) : Scene(manager) {
	GET_INSTANCE(ResourceManager, resourceManager);

	rcStgFrame_ = DxRect<int>(128, 16, 512, 464);

	{
		pTaskPlayer_ = std::make_shared<Stage_PlayerTask>(this);
		
		DxRect<int> rcMargin = DxRect<int>(12, 36, -12, -16);
		pTaskPlayer_->SetClip(rcStgFrame_ + rcMargin);

		pTaskPlayer_->SetX((rcStgFrame_.left + rcStgFrame_.right) / 2);
		pTaskPlayer_->SetY(rcStgFrame_.bottom - 80);

		this->AddTask(pTaskPlayer_);
	}

	bAutoDelete_ = false;
}
Stage_MainScene::~Stage_MainScene() {
}

void Stage_MainScene::Render() {
	Scene::Render();
}
void Stage_MainScene::Update() {
	Scene::Update();
}