#include "pch.h"

#include "StageMain.hpp"

#include "Player.hpp"
#include "Shot.hpp"

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

		resourceManager->LoadResource<TextureResource>(
			"resource/img/player/eff_ring.png", "img/player/eff_ring.png");
		resourceManager->LoadResource<TextureResource>(
			"resource/img/player/eff_burst.png", "img/player/eff_burst.png");
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
		objFrame_.SetSourceRect(DxRectangle(0, 0, 640, 480) - 1);
		objFrame_.SetDestRect(DxRectangle(0, 0, 640, 480));
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
class Stage_ShotTest : public TaskBase {
private:
	double angleA, angleB, angleC;
public:
	Stage_ShotTest(Scene* parent) : TaskBase(parent) {
		GET_INSTANCE(RandProvider, randGen);
		angleA = randGen->GetReal(0, GM_PI);
		angleB = 0;
		angleC = 0.0029269908;
	};

	virtual void Update() {
		Stage_MainScene* stage = (Stage_MainScene*)parent_;
		shared_ptr<Stage_ShotManager> shotManager = stage->GetShotManager();

		
		if (frame_ > 120 && frame_ % 2 == 0) {
			static size_t t = 0;
			
			for (size_t i = 0; i < 8; ++i) {
				bool bPolarityBlack = (t + i) % 10 >= 5;
				auto shot = shotManager->CreateShotA1(ShotOwnerType::Enemy,
					D3DXVECTOR2(320, 128), 2.6, angleA + GM_PI_X2 / 8 * i,
					bPolarityBlack ? ShotConst::RedScale : ShotConst::WhiteScale, 20);
				shotManager->AddEnemyShot(shot,
					bPolarityBlack ? IntersectPolarity::Black : IntersectPolarity::White);
			}

			angleB += angleC;
			angleA += angleB;
			++t;
		}
		
		/*
		if (frame_ > 120 && frame_ % 4 == 0) {
			static int c_gr = ShotConst::RedBallS;
			static double c_an = 0;

			for (int i = 0; i < 8; ++i) {
				shotManager->AddEnemyShot(shotManager->CreateShotA1(ShotOwnerType::Enemy,
					D3DXVECTOR2(320, 128), 2, GM_DTORA(360 / 16 + 360 / 8 * i + c_an), c_gr, 20),
					IntersectPolarity::White);
			}

			c_gr = (c_gr + 1) % (ShotConst::WhiteCrLaser + 1);
			c_an += 4.63;
		}
		*/

		++frame_;
	}
};

Stage_MainScene::Stage_MainScene(SceneManager* manager) : Scene(manager) {
	GET_INSTANCE(ResourceManager, resourceManager);

	rcStgFrame_ = DxRectangle<int>(128, 16, 512, 464);

	{
		pTaskPlayer_ = std::make_shared<Stage_PlayerTask>(this);
		pTaskPlayer_->pOwnRefWeak_ = weak_ptr(pTaskPlayer_);
		
		DxRectangle<int> rcMargin(12, 36, -12, -16);
		pTaskPlayer_->SetClip(rcStgFrame_ + rcMargin);

		pTaskPlayer_->SetX((rcStgFrame_.left + rcStgFrame_.right) / 2);
		pTaskPlayer_->SetY(rcStgFrame_.bottom - 80);

		this->AddTask(pTaskPlayer_);
	}
	{
		pTaskShotManager_ = std::make_shared<Stage_ShotManager>(this);

		DxRectangle<int> rcMargin(-64, -64, 64, 64);
		pTaskShotManager_->SetClip(DxRectangle<int>(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT) + rcMargin);

		pTaskShotManager_->LoadEnemyShotData();
		pTaskShotManager_->LoadPlayerShotData();

		this->AddTask(pTaskShotManager_);
	}
	{
		pTaskIntersectionManager_ = std::make_shared<Stage_IntersectionManager>(this);

		this->AddTask(pTaskIntersectionManager_);
	}

	{
		auto pTaskTest = std::make_shared<Stage_ShotTest>(this);
		this->AddTask(pTaskTest);
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