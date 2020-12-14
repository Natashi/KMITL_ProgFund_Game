#include "pch.h"

#include "StageMain.hpp"

#include "Player.hpp"
#include "Shot.hpp"

#include "Stage/BossMain.hpp"

//*******************************************************************
//Stage_SceneLoader
//*******************************************************************
CONSTRUCT_TASK(Stage_SceneLoader) {
	frameEnd_ = 30;

	SceneManager* manager = parent_->GetParentManager();

	{
		GET_INSTANCE(ResourceManager, resourceManager);

		//System
		SystemUtility::LoadSystemResources();

		//Stage UI
		resourceManager->LoadResource<TextureResource>("resource/img/stage/stg_frame.png", "img/stage/stg_frame.png");

		resourceManager->LoadResource<TextureResource>("resource/img/player/player_stat.png", "img/player/player_stat.png");

		//Stage main
		resourceManager->LoadResource<TextureResource>("resource/img/stage/enm_orb.png", "img/stage/enm_orb.png");

		resourceManager->LoadResource<TextureResource>("resource/img/stage/eff_aura.png", "img/stage/eff_aura.png");
		resourceManager->LoadResource<TextureResource>("resource/img/stage/eff_magicsquare.png", "img/stage/eff_magicsquare.png");

		resourceManager->LoadResource<TextureResource>("resource/img/stage/dot_shinki.png", "img/stage/dot_shinki.png");

		//Player
		resourceManager->LoadResource<TextureResource>("resource/img/player/eff_base.png", "img/player/eff_base.png");
		resourceManager->LoadResource<TextureResource>("resource/img/player/eff_dead.png", "img/player/eff_dead.png");
		resourceManager->LoadResource<TextureResource>("resource/img/player/eff_sloweffect.png", "img/player/eff_sloweffect.png");

		resourceManager->LoadResource<TextureResource>("resource/img/player/pl00.png", "img/player/pl00.png");

		resourceManager->LoadResource<TextureResource>("resource/img/player/eff_ring.png", "img/player/eff_ring.png");
		resourceManager->LoadResource<TextureResource>("resource/img/player/eff_burst.png", "img/player/eff_burst.png");
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
class StageUI_Score : public TaskBase {
	friend class Stage_MainSceneUI;
private:
	static constexpr const size_t COUNT_SCORE_DIGIT = 12U;
	static constexpr const size_t COUNT_SCORE_COMMA = (COUNT_SCORE_DIGIT - 1U) / 3U;
	static constexpr const size_t COUNT_SCORE_RENDER = COUNT_SCORE_DIGIT + COUNT_SCORE_COMMA;

	StaticRenderObject2D objScoreText_;
	DynamicRenderObject2D objScore_;
	D3DXVECTOR2 sizeAscii_;
	D3DXVECTOR2 sizeDigit_;
public:
	StageUI_Score(Scene* parent) : TaskBase(parent) {
		GET_INSTANCE(ResourceManager, resourceManager);

		auto textureAscii = resourceManager->GetResourceAs<TextureResource>("img/system/ascii.png");
		sizeAscii_ = D3DXVECTOR2(textureAscii->GetImageInfo()->Width, textureAscii->GetImageInfo()->Height);

		{
			objScoreText_.SetTexture(textureAscii);

			{
				std::string strText = "Score: ";
				std::vector<VertexTLX> vertex(strText.size() * 4U);
				std::vector<uint16_t> index(strText.size() * 6U);

				DxRectangle<int> rcBaseDst(0, 0, 16, 16);
				constexpr int xInc = 11;

				VertexTLX verts[4];
				for (size_t i = 0; i < strText.size(); ++i) {
					SystemUtility::SetVertexAsciiSingle(verts, strText[i], rcBaseDst, D3DXVECTOR2(14, 14), sizeAscii_);

					uint16_t biv = i * 4;
					uint16_t bii = i * 6;
					vertex[biv + 0] = verts[0]; vertex[biv + 1] = verts[1];
					vertex[biv + 2] = verts[2]; vertex[biv + 3] = verts[3];
					index[bii + 0] = biv + 0; index[bii + 1] = biv + 1; index[bii + 2] = biv + 2;
					index[bii + 3] = biv + 2; index[bii + 4] = biv + 1; index[bii + 5] = biv + 3;

					rcBaseDst += DxRectangle<int>::SetFromSize(xInc, 0);
				}

				objScoreText_.SetArrayVertex(vertex);
				objScoreText_.SetArrayIndex(index);

				objScoreText_.SetPosition(128 + 6, 0 + 2, 1);
			}
		}

		auto textureDigit = resourceManager->GetResourceAs<TextureResource>("img/system/sys_digit.png");
		sizeDigit_ = D3DXVECTOR2(textureDigit->GetImageInfo()->Width, textureDigit->GetImageInfo()->Height);

		objScore_.SetTexture(textureDigit);
		objScore_.SetPosition(224, -1, 1);
		{
			std::vector<uint16_t> index(COUNT_SCORE_RENDER * 6U);
			for (size_t i = 0; i < COUNT_SCORE_RENDER; ++i) {
				uint16_t biv = i * 4;
				uint16_t bii = i * 6;
				index[bii + 0] = biv + 0; index[bii + 1] = biv + 1; index[bii + 2] = biv + 2;
				index[bii + 3] = biv + 2; index[bii + 4] = biv + 1; index[bii + 5] = biv + 3;
			}
			objScore_.SetArrayIndex(index);
		}
	}

	virtual void Render(byte layer) {
		if (layer != 0) return;
		objScoreText_.Render();
		objScore_.Render();
	}
	virtual void Update() {
		shared_ptr<Stage_MainScene> stageScene = parent_->GetParentManager()->GetSceneAs<Stage_MainScene>(Scene::Stage);

		shared_ptr<Stage_PlayerTask> pTaskPlayer = stageScene->GetPlayer();
		PlayerData* playerData = pTaskPlayer->GetPlayerData();

		{
			int listDigits[COUNT_SCORE_DIGIT];

			{
				static std::unique_ptr<uint64_t> scoreMax = nullptr;
				if (scoreMax == nullptr) {
					uint64_t score = 1ui64;
					for (size_t i = 0; i < COUNT_SCORE_DIGIT; ++i)
						score *= 10ui64;
					scoreMax = std::unique_ptr<uint64_t>(new uint64_t(score));
				}

				uint64_t tScore = std::min(playerData->stat.score, *scoreMax);
				for (size_t i = 0; i < COUNT_SCORE_DIGIT; ++i) {
					listDigits[COUNT_SCORE_DIGIT - i - 1] = tScore % 10;
					tScore /= 10;
				}
			}

			std::vector<int> listDigitsRender(COUNT_SCORE_RENDER);
			{
				size_t iList = COUNT_SCORE_RENDER - 1;
				size_t iDigit = 0;
				while (true) {
					listDigitsRender[iList--] = listDigits[COUNT_SCORE_DIGIT - iDigit - 1];

					++iDigit;
					if (iDigit >= COUNT_SCORE_DIGIT) break;

					//Add comma
					if (iDigit % 3 == 0)
						listDigitsRender[iList--] = 12;
				}
			}

			{
				std::vector<VertexTLX> vertex(listDigitsRender.size() * 4U);

				DxRectangle<int> rcBaseDst(0, 0, 16, 20);

				VertexTLX verts[4];
				for (size_t i = 0; i < listDigitsRender.size(); ++i) {
					int iChar = listDigitsRender[i];
					bool bComma = iChar >= 10;

					DxRectangle<int> rcChar = DxRectangle<int>::SetFromIndex(16, 24, iChar, 20);

					SystemUtility::SetVertexAsciiSingle(verts, rcChar, 
						rcBaseDst + DxRectangle<int>::SetFromSize(bComma ? -5 : 0, 0), sizeDigit_);
					memcpy(vertex.data() + i * 4U, verts, sizeof(verts));

					rcBaseDst += DxRectangle<int>::SetFromSize(bComma ? 4 : 14, 0);
				}

				objScore_.SetArrayVertex(vertex);
			}
		}
	}
};
class StageUI_PlayerLife : public TaskBase {
	friend class Stage_MainSceneUI;
private:
	Sprite2D objLife_;
	std::vector<D3DXVECTOR2> listRenderPos_;
public:
	StageUI_PlayerLife(Scene* parent) : TaskBase(parent) {
		GET_INSTANCE(ResourceManager, resourceManager);

		{
			auto textureLife = resourceManager->GetResourceAs<TextureResource>("img/player/player_stat.png");

			objLife_.SetTexture(textureLife);
			objLife_.SetSourceRect(DxRectangle<int>(16 * 5, 0, 16 * 5 + 18, 16));
			objLife_.SetDestCenter();
			objLife_.UpdateVertexBuffer();
		}
	}

	virtual void Render(byte layer) {
		if (layer != 2) return;
		for (auto& iPos : listRenderPos_) {
			objLife_.SetPosition(iPos);
			objLife_.Render();
		}
	}
	virtual void Update() {
		shared_ptr<Stage_MainScene> stageScene = parent_->GetParentManager()->GetSceneAs<Stage_MainScene>(Scene::Stage);

		shared_ptr<Stage_PlayerTask> pTaskPlayer = stageScene->GetPlayer();
		PlayerData* playerData = pTaskPlayer->GetPlayerData();

		if (listRenderPos_.size() == playerData->life) return;
		if (playerData->life <= 0) {
			listRenderPos_.clear();
			return;
		}

		listRenderPos_.resize(playerData->life);

		constexpr float bx = 128 - 8 - 4;
		constexpr float by = 464 - 8 - 2;
		constexpr float ax = 0;
		constexpr float ay = -16;
		for (int i = 0; i < playerData->life; ++i)
			listRenderPos_[i] = D3DXVECTOR2(bx + ax * i, by + ay * i);
	}
};

Stage_MainSceneUI::Stage_MainSceneUI(SceneManager* manager) : Scene(manager) {
	GET_INSTANCE(ResourceManager, resourceManager);

	{
		auto textureFrame = resourceManager->GetResourceAs<TextureResource>("img/stage/stg_frame.png");
		objFrame_.SetTexture(textureFrame);
		objFrame_.SetSourceRect(DxRectangle(0, 0, 640, 480));
		objFrame_.SetDestRect(DxRectangle(0, 0, 640, 480));
		objFrame_.UpdateVertexBuffer();
	}
	objFrame_.SetBlendType(BlendMode::Invert);

	this->AddTask(std::make_shared<StageUI_Score>(this));
	this->AddTask(std::make_shared<StageUI_PlayerLife>(this));

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
		//auto pTaskTest = std::make_shared<Stage_ShotTest>(this);
		//this->AddTask(pTaskTest);
	}

	bAutoDelete_ = false;
}
Stage_MainScene::~Stage_MainScene() {
}

void Stage_MainScene::Render() {
	Scene::Render();
}
void Stage_MainScene::Update() {
	if (frame_ == 120) {
		auto pBossTask = shared_ptr<EnemyBoss_Shinki>(new EnemyBoss_Shinki(this));
		pBossTask->pOwnRefWeak_ = pBossTask;
		this->AddTask(pBossTask);
	}

	Scene::Update();
}