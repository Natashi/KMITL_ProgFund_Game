#include "pch.h"

#include "BossMain.hpp"

//----------------------------------------------------------------------------------------------------------

class Shinki_Non1 : public Stage_EnemyPhase {
	int step_;
	int tmpS_[2];
	double tmpD_[2];
public:
	Shinki_Non1(Scene* parent, Stage_EnemyTask_Scripted* objEnemy) : Stage_EnemyPhase(parent, objEnemy) {
		lifeMax_ = 2000;

		step_ = 0;
		tmpS_[0] = SystemUtility::RandDirection();
		tmpS_[1] = RandProvider::GetBase()->GetInt(0, 1);

		objEnemy->SetX(-999);
		objEnemy->SetY(-999);
		objEnemy->rateShotDamage_ = 0;
	}
	~Shinki_Non1() {
		Stage_MainScene* stage = (Stage_MainScene*)parent_;
		shared_ptr<Stage_ShotManager> shotManager = stage->GetShotManager();
		shotManager->DeleteInCircle(ShotOwnerType::Enemy, 320, 240, 1000, true);
	}

	virtual void Activate() {
		Stage_EnemyPhase::Activate();

		{
			parentEnemy_->SetX(92);
			parentEnemy_->SetY(-64);

			Stage_MovePatternLine_Frame* move = new Stage_MovePatternLine_Frame(parentEnemy_);
			move->SetAtFrame(320, 128, 60, Math::Lerp::MODE_DECELERATE);

			parentEnemy_->SetPattern(shared_ptr<Stage_MovePatternLine_Frame>(move));
		}
	}

	virtual void Update() {
		GET_INSTANCE(RandProvider, rand);
		Stage_MainScene* stage = (Stage_MainScene*)parent_;
		shared_ptr<Stage_ShotManager> shotManager = stage->GetShotManager();
		EnemyBoss_Shinki* objBoss = (EnemyBoss_Shinki*)parentEnemy_;

		switch (step_) {
		case 0:		//Initial wait
			if (frame_ == 100) {
				step_ = 1;
				frame_ = 0;
			}
			break;
		case 1:		//Fire circle
		{
			size_t eFrame = frame_ - 1;
			if (eFrame == 0) {
				objBoss->SetChargeType(0);
				objBoss->SetChargeDuration(24 * 2);
				tmpD_[0] = rand->GetReal(0, GM_PI_X2);
				tmpD_[1] = 0;
			}

			if (eFrame % 2 == 0) {
				double rFrame = (eFrame / 2) / 23.0;
				for (int i = 0; i < 32; ++i) {
					bool bPolar = i % 2 == 0;
					double sa = tmpD_[0] + GM_PI_X2 / 32 * i + tmpD_[1];
					double sr = Math::Lerp::Linear<double>(64, -32, rFrame);
					shotManager->AddEnemyShot(OffsetPos(parentEnemy_->GetMovePosition(), sa, sr),
						Math::Lerp::Linear<double>(0.5, 5.5, rFrame), sa,
						bPolar ? ShotConst::RedBallM : ShotConst::WhiteBallM, 8,
						bPolar ? IntersectPolarity::Black : IntersectPolarity::White);
				}
				tmpD_[1] += GM_DTORA(1.32) * tmpS_[0];
			}

			if (eFrame == 24 * 2 + 6) {
				step_ = 2;
				frame_ = 0;

				tmpS_[0] *= -1;
			}
			break;
		}
		case 2:		//Movement
		{
			objBoss->rateShotDamage_ = 1;

			size_t eFrame = frame_ - 1;
			if (eFrame == 0) {
				Stage_MovePatternLine_Frame* move = new Stage_MovePatternLine_Frame(parentEnemy_);
				move->SetAtFrame(320 + rand->GetReal(-160, 160), rand->GetReal(48, 96), 40, Math::Lerp::MODE_DECELERATE);
				parentEnemy_->SetPattern(move);
			}
			else if (eFrame == 80) {
				step_ = 3;
				frame_ = 0;
			}
			break;
		}
		case 3:		//Fire fans
		{
			struct Stack {
				bool pola;
				size_t cStack;
				float speed[2];
				Stack(bool p, size_t a, float b, float c) : pola(p), cStack(a) { speed[0] = b; speed[1] = c; }
			};
			struct Fan {
				size_t cWay;
				float gapAng;
				std::vector<Stack> listStackArg;
			};

			static std::vector<Fan> listFanArg;
			static size_t iFan;

			size_t eFrame = frame_ - 1;
			if (eFrame == 0) {
				objBoss->SetChargeType(1);
				objBoss->SetChargeDuration(100);

				if (listFanArg.size() == 0) {
					listFanArg = {
						{ 1, GM_DTORA(0), {
							Stack(true, 6, 3, 6) } },
						{ 2, GM_DTORA(40), { Stack(false, 8, 2.5, 5.5), 
							Stack(false, 8, 2.5, 5.5) } },
						{ 5, GM_DTORA(20), { Stack(false, 4, 2, 3.6), Stack(true, 6, 3, 5),
							Stack(false, 10, 4, 7.5), Stack(true, 6, 3, 5), Stack(false, 4, 2, 3.6) } },
					};
				}
				iFan = 0;
			}

			if (iFan < listFanArg.size() && eFrame == iFan * 30) {
				Fan* pFan = &listFanArg[iFan];

				double angToPlayer = parentEnemy_->GetDeltaAngle(stage->GetPlayer().get());
				float ang_off_way = (float)(pFan->cWay / 2U) - (pFan->cWay % 2U == 0U ? 0.5 : 0.0);
				for (size_t iWay = 0U; iWay < pFan->cWay; ++iWay) {
					float sa = angToPlayer + (iWay - ang_off_way) * pFan->gapAng;
					Stack* pStack = &pFan->listStackArg[iWay];
					for (size_t iStack = 0U; iStack < pStack->cStack; ++iStack) {
						bool bPol = (byte)pStack->pola ^ (byte)tmpS_[1];

						float ss = pStack->speed[0];
						if (pStack->cStack > 1U)
							ss += (pStack->speed[1] - pStack->speed[0]) * (iStack / (float)(pStack->cStack - 1U));

						shotManager->AddEnemyShot(parentEnemy_->GetMovePosition(), ss, sa,
							bPol ? ShotConst::WhiteScale : ShotConst::RedScale, 12,
							bPol ? IntersectPolarity::White : IntersectPolarity::Black);
					}
				}

				++iFan;
			}

			if (eFrame == listFanArg.size() * 30 + 2) {
				step_ = 4;
				frame_ = 0;

				tmpS_[1] = 1 - tmpS_[1];
			}
			break;
		}
		case 4:		//Movement + end
		{
			size_t eFrame = frame_ - 1;

			if (eFrame == 0) {
				Stage_MovePatternLine_Frame* move = new Stage_MovePatternLine_Frame(parentEnemy_);
				move->SetAtFrame(320 + rand->GetReal(-140, 140), rand->GetReal(64, 144), 40, Math::Lerp::MODE_DECELERATE);
				parentEnemy_->SetPattern(move);
			}
			else if (eFrame == 90) {
				step_ = 0;
				frame_ = 99;
			}
			break;
		}
		}

		Stage_EnemyPhase::Update();
	}
};

//----------------------------------------------------------------------------------------------------------

//*******************************************************************
//EnemyBoss_Shinki
//*******************************************************************
EnemyBoss_Shinki::EnemyBoss_Shinki(Scene* parent) : Stage_EnemyTask_Scripted(parent) {
	rateShotDamage_ = 0;

	frameAnimLeft_ = 0;
	frameAnimRight_ = 0;
	frameAnimCharge_ = 0;
	chargeTimer_ = 0;
	chargeMode_ = 0;

	pTaskCircle_ = nullptr;

	{
		GET_INSTANCE(ResourceManager, resourceManager);

		auto textureBoss = resourceManager->GetResourceAs<TextureResource>("img/stage/dot_shinki.png");

		objBossAnimation_.SetTexture(textureBoss);
		objBossAnimation_.SetSourceRect(DxRectangle<int>(0, 0, 128, 128));
		objBossAnimation_.SetDestCenter();
		objBossAnimation_.UpdateVertexBuffer();
	}

	{
		std::list<shared_ptr<Stage_EnemyPhase>> listPhase;

		listPhase.push_back(shared_ptr<Shinki_Non1>(new Shinki_Non1(parent_, this)));

		InitializePhases(listPhase);
	}
}

void EnemyBoss_Shinki::_RunAnimation() {
	constexpr int FRAME_I = 8;
	constexpr int FRAME_M = 7;

	static const int listSpriteIdle[] = { 0, 10, 20, 10, 0, 10, 20, 10, 0, 1, 11 };
	static const int listSpriteLeft[] = { 2, 2, 12, 12 };
	static const int listSpriteRight[] = { 22, 22, 32, 32 };
	static const std::vector<int> vecListSpriteCharge[] = {
		{ 4, 14, 24 },
		{ 5, 15, 25 },
	};
	static const int countSpriteIdle = sizeof(listSpriteIdle) / sizeof(int);
	static const int countSpriteLeft = sizeof(listSpriteLeft) / sizeof(int);
	static const int countSpriteRight = sizeof(listSpriteRight) / sizeof(int);
	static const int countVecSpriteCharge = sizeof(vecListSpriteCharge) / sizeof(std::vector<int>);

	int chargeMode = std::min<int>(chargeMode_, countVecSpriteCharge);

	double speedX = pattern_ ? pattern_->GetSpeedX() : 0;

	if (speedX < 0) {
		if (frameAnimLeft_ < countSpriteLeft * FRAME_M - 1)
			++frameAnimLeft_;
		frameAnimCharge_ = 0;
	}
	else if (frameAnimLeft_ > 0) {
		frameAnimLeft_ -= 2;
	}

	if (speedX > 0) {
		if (frameAnimRight_ < countSpriteRight * FRAME_M - 1)
			++frameAnimRight_;
		frameAnimCharge_ = 0;
	}
	else if (frameAnimRight_ > 0) {
		frameAnimRight_ -= 2;
	}

	if (chargeTimer_ > 0) {
		if (frameAnimCharge_ < vecListSpriteCharge[chargeMode].size() * FRAME_I - 1)
			++frameAnimCharge_;
		--chargeTimer_;
	}
	else if (frameAnimCharge_ > 0) {
		--frameAnimCharge_;
	}

	{
		int imgRender = 0;

		if (frameAnimLeft_ > 0) {
			int aFrame = (frameAnimLeft_ / FRAME_M) % countSpriteLeft;
			imgRender = listSpriteLeft[aFrame];
		}
		else if (frameAnimRight_ > 0) {
			int aFrame = (frameAnimRight_ / FRAME_M) % countSpriteRight;
			imgRender = listSpriteRight[aFrame];
		}
		else {		//Idle
			if (frameAnimCharge_ > 0) {
				const int* pSprites = vecListSpriteCharge[chargeMode].data();
				size_t maxCount = vecListSpriteCharge[chargeMode].size();
				imgRender = pSprites[(frameAnimCharge_ / FRAME_I) % maxCount];
			}
			else
				imgRender = listSpriteIdle[(frame_ / FRAME_I) % countSpriteIdle];
		}

		DxRectangle<int> frameRect = DxRectangle<int>::SetFromIndex(128, 128, imgRender, 10);
		objBossAnimation_.SetScroll(frameRect.left / 1024.0f, frameRect.top / 512.0f);
		//objSprite_.SetSourceRect(frameRect);
		//objSprite_.UpdateVertexBuffer();
	}

	objBossAnimation_.SetPosition(posX_, posY_ + sin(frame_ * GM_DTORA(2.37)) * 4, 1.0f);
}

void EnemyBoss_Shinki::Render(byte layer) {
	if (layer != LAYER_ENEMY) return;
	objBossAnimation_.Render();
}
void EnemyBoss_Shinki::Update() {
	if (frame_ == 60) {
		if (pTaskCircle_) pTaskCircle_->bEnd_ = true;
		pTaskCircle_ = shared_ptr<Boss_MagicCircle>(new Boss_MagicCircle(parent_, this));
		parent_->AddTask(pTaskCircle_);

		{
			auto stageUI = parent_->GetParentManager()->GetScene(Scene::StageUI).get();
			if (pTaskLifebar_) pTaskLifebar_->frameEnd_ = 0;
			pTaskLifebar_ = shared_ptr<Enemy_LifeIndicatorBoss>(new Enemy_LifeIndicatorBoss(stageUI, this));
			stageUI->AddTask(pTaskLifebar_);
		}
	}

	_Move();
	_RunAnimation();

	//Update intersection
	{
		auto intersectionManager = ((Stage_MainScene*)parent_)->GetIntersectionManager();
		{
			auto pTarget = new Stage_IntersectionTarget_Circle();
			pTarget->SetTargetType(Stage_IntersectionTarget::TypeTarget::EnemyToPlayer);
			pTarget->SetParent(pOwnRefWeak_);
			pTarget->SetCircle(DxCircle<float>(posX_, posY_, 24));
			pTarget->SetIntersectionSpace();
			intersectionManager->AddTarget(shared_ptr<Stage_IntersectionTarget>(pTarget));
		}
		{
			auto pTarget = new Stage_IntersectionTarget_Circle();
			pTarget->SetTargetType(Stage_IntersectionTarget::TypeTarget::EnemyToPlayerShot);
			pTarget->SetParent(pOwnRefWeak_);
			pTarget->SetCircle(DxCircle<float>(posX_, posY_, 48));
			pTarget->SetIntersectionSpace();
			intersectionManager->AddTarget(shared_ptr<Stage_IntersectionTarget>(pTarget));
		}
	}

	Stage_EnemyTask_Scripted::Update();
	++frame_;
}

//*******************************************************************
//Boss_MagicCircle
//*******************************************************************
Boss_MagicCircle::Boss_MagicCircle(Scene* parent, EnemyBoss_Shinki* objBoss) : TaskBase(parent) {
	objBoss_ = objBoss;

	bEnd_ = false;

	ZeroMemory(tAngle_, sizeof(tAngle_));
	ZeroMemory(tScale_, sizeof(tScale_));
	ZeroMemory(tAlpha_, sizeof(tAlpha_));

	{
		GET_INSTANCE(ResourceManager, resourceManager);

		objCircle_.SetTexture(resourceManager->GetResourceAs<TextureResource>("img/stage/eff_magicsquare.png"));
		objCircle_.SetSourceRectNormalized(DxRectangle<float>(0, 0, 1, 1));
		objCircle_.SetDestCenter();
		objCircle_.UpdateVertexBuffer();

		objCircle_.SetBlendType(BlendMode::Add);
	}
}
void Boss_MagicCircle::Render(byte layer) {
	if (layer != LAYER_ENEMY - 1) return;
	objCircle_.Render();
}
void Boss_MagicCircle::Update() {
	if (!bEnd_) {
		if (frame_ < 128) {
			double rate = frame_ / 127.0;
			tAngle_[1] = Math::Lerp::Decelerate<double>(6.3, 1, rate);
			tScale_[0] = Math::Lerp::Smooth<double>(0, 1.2, rate);
			tAlpha_[0] = Math::Lerp::Linear<double>(0, 1, rate);
		}
		else {
			tAngle_[1] = 1;
			tScale_[0] = 1.2;
			tAlpha_[0] = 1;
		}
	}
	else {
		if (frameEnd_ == UINT_MAX) {
			frameEnd_ = 90;
			frame_ = 0;
			tScale_[1] = tScale_[0];
			tAlpha_[1] = tAlpha_[0];
		}

		double rate = frame_ / 89.0;
		tScale_[0] = Math::Lerp::Accelerate<double>(tScale_[1], 0, rate);
		tAlpha_[0] = Math::Lerp::Smooth<double>(tAlpha_[2], 0, rate);
	}

	{
		double sFrame = tAngle_[0];
		double angleZ = sFrame * GM_DTORA(1.12);
		double angleX = GM_DTORA(36) * cos(sFrame * GM_DTORA(0.842));
		double angleY = sin(sFrame * GM_DTORA(0.711));

		objCircle_.SetAngle(angleX, GM_DTORA(33) * angleY * angleY, angleZ);
	}

	objCircle_.SetPosition(objBoss_->GetMovePosition());
	objCircle_.SetScale(tScale_[0], tScale_[0], 1.0f);
	objCircle_.SetAlpha(tAlpha_[0] * 128);

	tAngle_[0] += tAngle_[1];
	++frame_;
}

//*******************************************************************
//Enemy_LifeIndicatorBoss
//*******************************************************************
Enemy_LifeIndicatorBoss::Enemy_LifeIndicatorBoss(Scene* parent, Stage_EnemyTask_Scripted* objEnemy) : TaskBase(parent) {
	objEnemy_ = objEnemy;

	prevScale_ = 0;

	{
		std::vector<VertexTLX> vertex;
		vertex.resize(6);

		D3DCOLOR colorCen = D3DCOLOR_XRGB(255, 255, 255);
		D3DCOLOR colorEnd = D3DCOLOR_XRGB(192, 192, 192);

		vertex[0] = VertexTLX(D3DXVECTOR3(-192, 0, 1), D3DXVECTOR2(), colorEnd);
		vertex[1] = VertexTLX(D3DXVECTOR3(-192, BAR_WD, 1), D3DXVECTOR2(), colorEnd);

		vertex[2] = VertexTLX(D3DXVECTOR3(0, 0, 1), D3DXVECTOR2(), colorCen);
		vertex[3] = VertexTLX(D3DXVECTOR3(0, BAR_WD, 1), D3DXVECTOR2(), colorCen);

		vertex[4] = VertexTLX(D3DXVECTOR3(192, 0, 1), D3DXVECTOR2(), colorEnd);
		vertex[5] = VertexTLX(D3DXVECTOR3(192, BAR_WD, 1), D3DXVECTOR2(), colorEnd);

		objBar_.SetPrimitiveType(D3DPT_TRIANGLESTRIP);
		objBar_.SetArrayVertex(vertex);
	}
}

void Enemy_LifeIndicatorBoss::Render(byte layer) {
	if (layer != 1) return;

	auto stage = (Stage_MainScene*)(parent_->GetParentManager()->GetScene(Scene::Stage).get());
	auto objPlayer = stage->GetPlayer();

	objBar_.SetPosition(320, 16, 1);
	objBar_.SetAlpha(objPlayer->posY_ < 16 + BAR_WD + 24 ? 96 : 255);
	objBar_.Render();
	objBar_.SetPosition(320, 464 + 2, 1);
	objBar_.SetAlpha(255);
	objBar_.Render();
}
void Enemy_LifeIndicatorBoss::Update() {
	auto stage = (Stage_MainScene*)(parent_->GetParentManager()->GetScene(Scene::Stage).get());

	double targetScale = 0;
	if (auto pPhase = objEnemy_->GetCurrentPhase()) {
		targetScale = objEnemy_->life_ / pPhase->lifeMax_;
	}

	prevScale_ = prevScale_ * 0.9 + targetScale * 0.1;
	objBar_.SetScaleX(std::clamp<double>(prevScale_, 0, 1));

	{
		D3DXVECTOR3 color;
		if (prevScale_ > 0.5) {
			double rate = (prevScale_ - 0.5) * 2;
			color.x = Math::Lerp::Smooth(248, 78, rate);
			color.y = Math::Lerp::Smooth(186, 255, rate);
			color.z = Math::Lerp::Smooth(44, 121, rate);
		}
		else {
			double rate = prevScale_ * 2;
			color.x = Math::Lerp::Smooth(255, 248, rate);
			color.y = Math::Lerp::Smooth(4, 186, rate);
			color.z = Math::Lerp::Smooth(4, 44, rate);
		}
		objBar_.SetColor(color / 255.0f);
	}
}
