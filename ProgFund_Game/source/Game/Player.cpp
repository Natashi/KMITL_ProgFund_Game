#include "pch.h"
#include "Player.hpp"

#include "Shot.hpp"
#include "StageMain.hpp"

//*******************************************************************
//Stage_PlayerTask
//*******************************************************************
CONSTRUCT_TASK(Stage_PlayerTask) {
	playerData_.life = 2;

	rcClip_ = DxRectangle<int>(0, 0, 640, 480);

	frameAnimLeft_ = 0;
	frameAnimRight_ = 0;

	tendencyPolarity_ = 0;
	scalePolarity_ = 0;

	frameInvincibility_ = 90;
	bFocus_ = false;

	{
		GET_INSTANCE(ResourceManager, resourceManager);

		auto texturePlayer = resourceManager->GetResourceAs<TextureResource>("img/player/pl00.png");

		objSprite_.SetTexture(texturePlayer);
		objSprite_.SetSourceRect(DxRectangle(0, 0, 32, 48));
		objSprite_.SetDestCenter();
		objSprite_.UpdateVertexBuffer();
	}

	pPatternAngle_ = new Stage_MovePatternAngle(this);
	pattern_ = shared_ptr<Stage_MovePattern>(pPatternAngle_);

	_ChangePolarity();
}
Stage_PlayerTask::~Stage_PlayerTask() {
}

void Stage_PlayerTask::_Move() {
	GET_INSTANCE(InputManager, inputManager);

	double sx = 0;
	double sy = 0;

	KeyState stateSlow = inputManager->GetKeyState(VirtualKey::Focus);
	bFocus_ = stateSlow == KeyState::Push || stateSlow == KeyState::Hold;

	if (DIJOYSTATE* statePad = inputManager->GetPadState(0)) {
		if (statePad->lX != 0 || statePad->lY != 0) {
			//double rateX = std::clamp<double>(statePad->lX / 750.0, -1, 1);		//Left analog X
			//double rateY = std::clamp<double>(statePad->lY / 750.0, -1, 1);		//Left analog Y
			double rateX = statePad->lX > 200 ? 1 : (statePad->lX < -200 ? -1 : 0);
			double rateY = statePad->lY > 200 ? 1 : (statePad->lY < -200 ? -1 : 0);

			double speedMax = bFocus_ ? SPEED_SLOW : SPEED_FAST;

			sx = speedMax * rateX;
			sy = speedMax * rateY;

			pPatternAngle_->speed_ = hypot(sx, sy);
			if (pPatternAngle_->speed_ != 0)
				pPatternAngle_->angDirection_ = atan2(sy, sx);
		}
	}

	{
		KeyState stateLeft = inputManager->GetKeyState(VirtualKey::Left);
		KeyState stateRight = inputManager->GetKeyState(VirtualKey::Right);
		KeyState stateUp = inputManager->GetKeyState(VirtualKey::Up);
		KeyState stateDown = inputManager->GetKeyState(VirtualKey::Down);

		bool bKeyLeft = stateLeft == KeyState::Push || stateLeft == KeyState::Hold;
		bool bKeyRight = stateRight == KeyState::Push || stateRight == KeyState::Hold;
		bool bKeyUp = stateUp == KeyState::Push || stateUp == KeyState::Hold;
		bool bKeyDown = stateDown == KeyState::Push || stateDown == KeyState::Hold;

		if (bKeyLeft || bKeyRight || bKeyUp || bKeyDown) {
			if (bKeyLeft || bKeyRight) sx = 0;
			if (bKeyUp || bKeyDown) sy = 0;

			double speed = bFocus_ ? SPEED_SLOW : SPEED_FAST;

			if (bKeyLeft && !bKeyRight) sx -= speed;
			if (!bKeyLeft && bKeyRight) sx += speed;
			if (bKeyUp && !bKeyDown) sy -= speed;
			if (!bKeyUp && bKeyDown) sy += speed;

			pPatternAngle_->speed_ = speed;
			if (sx != 0 && sy != 0) {	//Normalize axes
				constexpr double diagFactor = 1.0 / GM_SQRT2;
				sx *= diagFactor;
				sy *= diagFactor;

				if (sx > 0)
					pPatternAngle_->angDirection_ = Math::DegreeToRadian(sy > 0 ? 45 : 315);
				else
					pPatternAngle_->angDirection_ = Math::DegreeToRadian(sy > 0 ? 135 : 225);
			}
			else if (sx != 0 || sy != 0) {
				if (sx != 0)
					pPatternAngle_->angDirection_ = Math::DegreeToRadian(sx > 0 ? 0 : 180);
				else if (sy != 0)
					pPatternAngle_->angDirection_ = Math::DegreeToRadian(sy > 0 ? 90 : 270);
			}
			else {
				pPatternAngle_->speed_ = 0;
			}
		}
	}

	pPatternAngle_->c_ = sx;
	pPatternAngle_->s_ = sy;

	//Add and clip player position
	{
		posX_ = std::clamp<float>(posX_ + sx, rcClip_.left, rcClip_.right);
		posY_ = std::clamp<float>(posY_ + sy, rcClip_.top, rcClip_.bottom);
	}
}
void Stage_PlayerTask::_RunAnimation() {
	constexpr int FRAME_I = 6;
	constexpr int FRAME_M = 4;

	static const int listSpriteIdle[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	static const int listSpriteLeft[] = { 10, 11, 12, 13, 14, 15, 15, 16, 16, 17, 17 };
	static const int listSpriteRight[] = { 20, 21, 22, 23, 24, 25, 25, 26, 26, 27, 27 };
	static const int countSpriteIdle = sizeof(listSpriteIdle) / sizeof(int);
	static const int countSpriteLeft = sizeof(listSpriteLeft) / sizeof(int);
	static const int countSpriteRight = sizeof(listSpriteRight) / sizeof(int);
	static const int resetFrameLeft = 4 * FRAME_M;
	static const int resetFrameRight = 4 * FRAME_M;

	if (pPatternAngle_->c_ < 0) {
		if (frameAnimLeft_ < countSpriteLeft * FRAME_M - 1)
			++frameAnimLeft_;
		else
			frameAnimLeft_ = resetFrameLeft;
	}
	else if (frameAnimLeft_ > 0) {
		if (frameAnimLeft_ > resetFrameLeft)
			frameAnimLeft_ = resetFrameLeft;
		frameAnimLeft_ -= 2;
	}

	if (pPatternAngle_->c_ > 0) {
		if (frameAnimRight_ < countSpriteRight * FRAME_M - 1)
			++frameAnimRight_;
		else
			frameAnimRight_ = resetFrameRight;
	}
	else if (frameAnimRight_ > 0) {
		if (frameAnimRight_ > resetFrameRight)
			frameAnimRight_ = resetFrameRight;
		frameAnimRight_ -= 2;
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
			int aFrame = (frame_ / FRAME_I) % countSpriteIdle;
			imgRender = listSpriteIdle[aFrame];
		}

		DxRectangle<int> frameRect = DxRectangle<int>::SetFromIndex(32, 48, imgRender, 10);
		objSprite_.SetScroll(frameRect.left / 256.0, frameRect.top / 256.0);
		//objSprite_.SetSourceRect(frameRect);
		//objSprite_.UpdateVertexBuffer();
	}
	
	if (frameInvincibility_ > 0) {
		objSprite_.SetColor((frameInvincibility_ % 4) >= 2 ? 
			D3DCOLOR_XRGB(32, 32, 255) : 0x00ffffff);
	}
	else {
		objSprite_.SetColor(0x00ffffff);
	}
}
void Stage_PlayerTask::_Shoot() {
	GET_INSTANCE(InputManager, inputManager);

	KeyState stateShot = inputManager->GetKeyState(VirtualKey::Shot);

	static size_t sFrame = 0;

	if (stateShot == KeyState::Push || stateShot == KeyState::Hold) {
		auto shotManager = ((Stage_MainScene*)parent_)->GetShotManager();

		auto AddShot = [&](int index, D3DXVECTOR2 pos, float sp, double ang, int gr, double damage) {
			if (index < 0) pos += D3DXVECTOR2(posX_, posY_);
			else {
				if (taskOption_[index] == nullptr) return;
				pos += taskOption_[index]->GetPosition();
			}

			auto shot = shotManager->CreateShotA1(ShotOwnerType::Player,
				pos, sp, -GM_PI_2 + GM_DTORA(ang), gr, 0);
			shot->SetAngleZOff(-GM_PI_2);
			shot->damage_ = damage;
			shotManager->AddPlayerShot(shot, IntersectPolarity::White);
		};

		if (frame_ % 6 == 0) {
			AddShot(-1, D3DXVECTOR2(10, 32), 48, 0, ShotPlayerConst::Main, 2);
			AddShot(-1, D3DXVECTOR2(-10, 32), 48, 0, ShotPlayerConst::Main, 2);
		}

		if (!bFocus_) {
			if (frame_ % 6) return;
			//double angOff = Math::DegreeToRadian(8 + sFrame * 8);
			AddShot(0, D3DXVECTOR2(0, 1), 22, -4, ShotPlayerConst::Needle, 5);
			AddShot(0, D3DXVECTOR2(0, 1), 22, 7, ShotPlayerConst::Needle, 5);
			AddShot(1, D3DXVECTOR2(0, 1), 22, -7, ShotPlayerConst::Needle, 5);
			AddShot(1, D3DXVECTOR2(0, 1), 22, 4, ShotPlayerConst::Needle, 5);
			sFrame = (sFrame + 1) % 5;
		}
		else {
			if (frame_ % 5) return;
			AddShot(0, D3DXVECTOR2(-9, 4), 28, 0, ShotPlayerConst::Needle, 3);
			AddShot(0, D3DXVECTOR2(8, 4), 28, 0, ShotPlayerConst::Needle, 3);

			AddShot(1, D3DXVECTOR2(-8, 4), 28, 0, ShotPlayerConst::Needle, 3);
			AddShot(1, D3DXVECTOR2(9, 4), 28, 0, ShotPlayerConst::Needle, 3);
		}
	}
}

void Stage_PlayerTask::_ChangePolarity() {
	static D3DCOLOR listColor[] = { 0xffffffff, D3DCOLOR_XRGB(159, 171, 207), D3DCOLOR_XRGB(220, 48, 48) };

	if (taskBarrier_ == nullptr) {
		polarity_ = IntersectPolarity::White;
		tendencyPolarity_ = 1;
		scalePolarity_ = 1.0;
	}
	else {
		if (polarity_ == IntersectPolarity::White) {
			polarity_ = IntersectPolarity::Black;
			tendencyPolarity_ = -1;
			scalePolarity_ = POLARITY_SWITCH;
		}
		else {
			polarity_ = IntersectPolarity::White;
			tendencyPolarity_ = 1;
			scalePolarity_ = -POLARITY_SWITCH;
		}
	}

	if (taskBarrier_) taskBarrier_->bEnd_ = true;
	taskBarrier_ = shared_ptr<Player_BarrierTask>(
		new Player_BarrierTask(parent_, listColor[(int)polarity_]));
	parent_->AddTask(taskBarrier_);
}
void Stage_PlayerTask::_Death() {
	frameInvincibility_ = IFRAME_DEATH;

	playerData_.countAbsorb = 0;
	playerData_.scoreMultiplier = 1;
	playerData_.specialCharge /= 2;

	playerData_.life -= 1;

	parent_->AddTask(new Player_DeadManagerTask(parent_));
}

void Stage_PlayerTask::Render(byte layer) {
	if (layer != LAYER_PLAYER) return;
	objSprite_.Render();
}
void Stage_PlayerTask::Update() {
	GET_INSTANCE(InputManager, inputManager);

	KeyState stateShot = inputManager->GetKeyState(VirtualKey::Shot);
	KeyState stateSlow = inputManager->GetKeyState(VirtualKey::Focus);
	KeyState stateSpell = inputManager->GetKeyState(VirtualKey::Spell);

	bFocus_ = stateSlow == KeyState::Push || stateSlow == KeyState::Hold;

	//Update movement
	_Move();
	objSprite_.SetPosition(roundf(posX_), roundf(posY_), 1.0f);

	//Update player animation
	_RunAnimation();

	//Shoot stuff
	_Shoot();
	
	//Increase multiplier
	{
		if (frameInvincibility_ == 0 && frame_ % 15 == 0) {
			if (playerData_.scoreMultiplier < 64)
				playerData_.scoreMultiplier += 0.1;
		}
	}

	//Change polarity
	{
		if ((stateSpell == KeyState::Push) && (abs(scalePolarity_) >= POLARITY_COOLDOWN))
			_ChangePolarity();

		scalePolarity_ += POLARITY_INC_STEP * tendencyPolarity_;
	}

	//Load intersection
	{
		auto intersectionManager = ((Stage_MainScene*)parent_)->GetIntersectionManager();

		//Graze
		{
			auto pTarget = shared_ptr<Stage_IntersectionTarget_Player>(new Stage_IntersectionTarget_Player(true));
			pTarget->SetParent(pOwnRefWeak_);
			pTarget->SetCircle(DxCircle<float>(posX_, posY_,
				frameInvincibility_ == 0 ? BARRIER_SIZE : BARRIER_SIZE_MIN));
			pTarget->SetIntersectionSpace();
			intersectionManager->AddTarget(Stage_IntersectionTarget::TypeTarget::Player, pTarget);
		}

		//Hit
		if (frameInvincibility_ == 0) {
			auto pTarget = shared_ptr<Stage_IntersectionTarget_Player>(new Stage_IntersectionTarget_Player(false));
			pTarget->SetParent(pOwnRefWeak_);
			pTarget->SetCircle(DxCircle<float>(posX_, posY_, HITBOX_SIZE));
			pTarget->SetIntersectionSpace();
			intersectionManager->AddTarget(Stage_IntersectionTarget::TypeTarget::Player, pTarget);
		}
	}
	if (frameInvincibility_ > 0) --frameInvincibility_;

	//Run hitbox graphic tasks
	{
		/*
		if (bFocus_) {
			if (*taskHitbox_ == nullptr) {
				taskHitbox_[0] = shared_ptr<Player_HitboxTask>(
					new Player_HitboxTask(parent_, 255, 160, 0.3, 12, -180, 180, 3));
				taskHitbox_[1] = shared_ptr<Player_HitboxTask>(
					new Player_HitboxTask(parent_, 0, 255, 1.2, 18, 0, 0, -2));

				parent_->AddTask(taskHitbox_[0]);
				parent_->AddTask(taskHitbox_[1]);
			}
		}
		else {
			if (*taskHitbox_ != nullptr) {
				taskHitbox_[0]->bEnd_ = true;
				taskHitbox_[1]->bEnd_ = true;

				taskHitbox_[0] = nullptr;		//Removes a reference
				taskHitbox_[1] = nullptr;
			}
		}
		*/
	}

	//Run option graphic tasks
	{
		if (*taskOption_ == nullptr) {
			taskOption_[0] = shared_ptr<Player_OptionTask>(
				new Player_OptionTask(parent_, D3DXVECTOR2(-36, 36), D3DXVECTOR2(-15, -32), 1));
			taskOption_[1] = shared_ptr<Player_OptionTask>(
				new Player_OptionTask(parent_, D3DXVECTOR2(36, 36), D3DXVECTOR2(15, -32), -1));

			parent_->AddTask(taskOption_[0]);
			parent_->AddTask(taskOption_[1]);
		}
	}

	++frame_;
}

void Stage_PlayerTask::Intersect(shared_ptr<Stage_IntersectionTarget> ownTarget, shared_ptr<Stage_IntersectionTarget> otherTarget) {
	shared_ptr<Stage_ObjCollision> pOther = otherTarget->GetParent().lock();

	if (auto pOwnTarget = dynamic_cast<Stage_IntersectionTarget_Player*>(ownTarget.get())) {
		switch (otherTarget->GetTargetType()) {
		case Stage_IntersectionTarget::TypeTarget::EnemyToPlayer:
			//if (auto pEnemy = dynamic_cast<Stage_IntersectionTarget_Player*>(ownTarget.get())) {
				if (!pOwnTarget->IsGraze()) {
					_Death();
				}
			//}
			break;
		case Stage_IntersectionTarget::TypeTarget::EnemyShot:
			if (auto pShot = dynamic_cast<Stage_ObjShot*>(otherTarget->GetParent().lock().get())) {
				if (!pOwnTarget->IsGraze()) {
					if (pShot->GetPolarity() != GetPolarity())
						_Death();
				}
				else {
					if (pShot->GetPolarity() == GetPolarity()) {
						double mul = (int)(playerData_.scoreMultiplier * 10) / 10.0;	//Round to 1 decimal place
						playerData_.stat.score += 100 * mul;

						++(playerData_.countAbsorb);
						++(playerData_.stat.totalAbsorb);
						if (playerData_.stat.maxAbsorb < playerData_.countAbsorb)
							playerData_.stat.maxAbsorb = playerData_.countAbsorb;
					}
				}
			}
			break;
		}
	}
}

//*******************************************************************
//Player_HitboxTask
//*******************************************************************
Player_HitboxTask::Player_HitboxTask(Scene* parent, int s_alpha, int e_alpha, double s_scale,
	size_t s_timer, double s_angle, double e_angle, double r_spin) : TaskBase(parent)
{
	frameSub_ = 0;
	bEnd_ = false;

	iniAlpha_ = s_alpha;
	endAlpha_ = e_alpha;
	iniScale_ = s_scale;
	scaleTimer_ = s_timer;
	iniAngle1_ = GM_DTORA(s_angle);
	iniAngle2_ = GM_DTORA(e_angle);
	spinRate_ = GM_DTORA(r_spin);

	tScale[0] = iniScale_;
	tScale[1] = tScale[0];
	tAngle = iniAngle1_;

	{
		GET_INSTANCE(ResourceManager, resourceManager);

		auto textureHitbox = resourceManager->GetResourceAs<TextureResource>("img/player/eff_sloweffect.png");

		objHitbox_.SetTexture(textureHitbox);
		objHitbox_.SetSourceRect(DxRectangle(0, 0, 63, 63));
		objHitbox_.SetDestCenter();
		objHitbox_.UpdateVertexBuffer();
	}

	objHitbox_.SetAlpha(iniAlpha_);
	objHitbox_.SetScale(0, 0, 1.0f);
	objHitbox_.SetAngleZ(0);
}

void Player_HitboxTask::Render(byte layer) {
	if (layer != LAYER_SHOT + 1) return;
	objHitbox_.Render();
}
void Player_HitboxTask::Update() {
	if (!bEnd_) {
		if (frame_ < 18) {
			tScale[0] = Math::Lerp::Smooth<double>(iniScale_, 1, std::min(frame_, scaleTimer_) / (double)scaleTimer_);
			tAngle = Math::Lerp::Linear(iniAngle1_, iniAngle2_, frame_ / 17.0);

			objHitbox_.SetAlpha(Math::Lerp::Linear(iniAlpha_, endAlpha_, std::min(frame_, 6U) / 6.0));
		}
		else {
			if (frame_ == 18) objHitbox_.SetAlpha(endAlpha_);
			tAngle += spinRate_;
		}
	}
	else {
		if (frameEnd_ == UINT_MAX) {
			frameEnd_ = 6;
			frame_ = 0;
			tScale[1] = tScale[0];
		}

		tScale[0] = Math::Lerp::Accelerate<double>(tScale[1], 0, frame_ / 5.0);
		tAngle += spinRate_;
	}

	{
		auto objPlayer = ((Stage_MainScene*)parent_)->GetPlayer();
		objHitbox_.SetX(roundf(objPlayer->GetX()));
		objHitbox_.SetY(roundf(objPlayer->GetY()));
	}
	objHitbox_.SetScale(tScale[0], tScale[0], 1.0f);
	objHitbox_.SetAngleZ(tAngle);

	++frame_;
}

//*******************************************************************
//Player_BarrierTask
//*******************************************************************
Player_BarrierTask::Player_BarrierTask(Scene* parent, D3DCOLOR color) : TaskBase(parent) {
	frameSub_ = 0;
	bEnd_ = false;

	color_ = 0xffffffff;
	ZeroMemory(tScale, sizeof(tScale));
	ZeroMemory(tAlpha, sizeof(tAlpha));

	{
		GET_INSTANCE(ResourceManager, resourceManager);

		auto textureBorder = resourceManager->GetResourceAs<TextureResource>("img/player/eff_ring.png");
		auto textureGlow = resourceManager->GetResourceAs<TextureResource>("img/player/eff_burst.png");

		objBorder_.SetTexture(textureBorder);
		objBorder_.SetSourceRectNormalized(DxRectangle<float>(0, 0, 1, 1));
		objBorder_.SetDestCenter();
		objBorder_.UpdateVertexBuffer();

		objGlow_.SetTexture(textureGlow);
		objGlow_.SetSourceRectNormalized(DxRectangle<float>(0, 0, 1, 1));
		objGlow_.SetDestCenter();
		objGlow_.UpdateVertexBuffer();

		objBorder_.SetBlendType(BlendMode::Add);
		objGlow_.SetBlendType(BlendMode::Add);
		objBorder_.SetColor(color);
		objGlow_.SetColor(color);
	}
}

void Player_BarrierTask::Render(byte layer) {
	if (layer != LAYER_PLAYER - 1) return;
	objBorder_.Render();
	objGlow_.Render();
}
void Player_BarrierTask::Update() {
	if (!bEnd_) {
		if (frame_ < 30) {
			double rate = frame_ / 29.0;
			tScale[0] = Math::Lerp::Smooth<double>(0.2, 1, rate);
			tAlpha[0] = std::min(Math::Lerp::Linear<double>(0, 2, rate), 1.0);
			tAlpha[1] = Math::Lerp::Decelerate<double>(0, 1, rate);

			double angleX = Math::Lerp::Smooth<double>(GM_DTORA(231), 0, rate);
			double angleY = Math::Lerp::Linear<double>(GM_DTORA(144), 0, rate);
			objBorder_.SetAngle(angleX, angleY, 0);
		}
		else {
			tScale[0] = 1;
			tAlpha[0] = 1;
			tAlpha[1] = 1;

			double angleZ = frameSub_ * GM_DTORA(0.93);
			double angleX = GM_DTORA(43) * sin(frameSub_ * GM_DTORA(1.242));
			double angleY = sin(frameSub_ * GM_DTORA(1.411));

			objBorder_.SetAngle(angleX, GM_DTORA(33) * angleY, 0);

			++frameSub_;
		}
	}
	else {
		if (frameEnd_ == UINT_MAX) {
			frameEnd_ = 12;
			frame_ = 0;
			tScale[1] = tScale[0];
			tAlpha[2] = tAlpha[0];
			tAlpha[3] = tAlpha[1];
		}

		double rate = frame_ / 11.0;
		tScale[0] = Math::Lerp::Accelerate<double>(tScale[1], 0, rate);
		tAlpha[0] = Math::Lerp::Smooth<double>(tAlpha[2], 0, rate);
		tAlpha[1] = Math::Lerp::Smooth<double>(tAlpha[3], 0, rate);
	}

	{
		auto objPlayer = ((Stage_MainScene*)parent_)->GetPlayer();
		objBorder_.SetPosition(objPlayer->GetMovePosition());
		objGlow_.SetPosition(objPlayer->GetMovePosition());
	}

	static const double BORDER_SC = Stage_PlayerTask::BARRIER_SIZE / (double)32 * 1.5;
	static const double GLOW_SC = Stage_PlayerTask::BARRIER_SIZE / (double)128;

	objBorder_.SetScale(tScale[0] * BORDER_SC, tScale[0] * BORDER_SC, 1.0f);
	objGlow_.SetScale(tScale[0] * GLOW_SC, tScale[0] * GLOW_SC, 1.0f);
	objBorder_.SetAlpha(tAlpha[0] * 128);
	objGlow_.SetAlpha(tAlpha[1] * 255);

	++frame_;
}

//*******************************************************************
//Player_OptionTask
//*******************************************************************
Player_OptionTask::Player_OptionTask(Scene* parent, 
	CD3DXVECTOR2 posUF, CD3DXVECTOR2 posF, double dir) : TaskBase(parent)
{
	frameSub_ = 0;
	bEnd_ = false;

	posUnFocus_ = posUF;
	posFocus_ = posF;
	direction_ = dir;

	{
		GET_INSTANCE(ResourceManager, resourceManager);

		auto textureOption = resourceManager->GetResourceAs<TextureResource>("img/player/pl00.png");

		objOption_.SetTexture(textureOption);
		objOption_.SetSourceRect(DxRectangle<float>::SetFromSize(64, 144, 15, 15));
		objOption_.SetDestCenter();
		objOption_.UpdateVertexBuffer();
	}

	{
		auto objPlayer = ((Stage_MainScene*)parent_)->GetPlayer();
		position_ = objPlayer->GetMovePosition();
	}
}

void Player_OptionTask::Render(byte layer) {
	if (layer != LAYER_OPTION) return;
	objOption_.Render();
}
void Player_OptionTask::Update() {
	double tScale = 1.0;

	if (!bEnd_) {
		if (frame_ < 16) {
			tScale = Math::Lerp::Smooth<double>(0, 1, frame_ / 15.0);
		}
		else {
			tScale = 1;
		}

		auto objPlayer = ((Stage_MainScene*)parent_)->GetPlayer();

		float tx = objPlayer->GetX();
		float ty = objPlayer->GetY();
		if (objPlayer->IsFocus()) {
			tx += posFocus_.x;
			ty += posFocus_.y;
		}
		else {
			tx += posUnFocus_.x;
			ty += posUnFocus_.y;
		}

		position_.x = Math::Lerp::Linear(position_.x, tx, 0.24);
		position_.y = Math::Lerp::Linear(position_.y, ty, 0.24);
	}
	else {
		static double lScale;
		if (frameEnd_ == UINT_MAX) {
			frameEnd_ = frame_ + 12;
			lScale = tScale;
		}

		tScale = Math::Lerp::Smooth<double>(lScale, 0, frameSub_ / 12.0);

		++frameSub_;
	}

	objOption_.SetPosition(position_ - D3DXVECTOR2(1, 1));
	objOption_.SetScale(tScale, tScale, 1.0);
	objOption_.SetAngleZ(frame_ * Math::DegreeToRadian(2.7) * direction_);

	++frame_;
}

//*******************************************************************
//Player_DeadManagerTask
//*******************************************************************
Player_DeadManagerTask::Player_DeadManagerTask(Scene* parent) : TaskBase(parent) {
}
void Player_DeadManagerTask::Update() {
	static D3DXVECTOR2 posInvert;

	constexpr size_t DU_INV = 110;
	constexpr size_t WT_1 = 4;
	constexpr size_t WT_2 = 16;
	constexpr size_t WT_3 = 20;

	switch (frame_) {
	case 0:
		posInvert = ((Stage_MainScene*)parent_)->GetPlayer()->GetMovePosition();
		parent_->AddTask(new Player_DeadCircleTask(parent_, posInvert));
		parent_->AddTask(new Player_DeadParticleTask(parent_, posInvert));
		break;
	case WT_1:
		posInvert = ((Stage_MainScene*)parent_)->GetPlayer()->GetMovePosition();
		parent_->AddTask(new Player_DeadInvertTask(parent_, 1100, DU_INV, posInvert));
		break;
	case WT_1 + WT_2:
		parent_->AddTask(new Player_DeadInvertTask(parent_, 900, DU_INV - WT_2, posInvert + D3DXVECTOR2(-32, -32)));
		parent_->AddTask(new Player_DeadInvertTask(parent_, 900, DU_INV - WT_2, posInvert + D3DXVECTOR2(32, -32)));
		parent_->AddTask(new Player_DeadInvertTask(parent_, 900, DU_INV - WT_2, posInvert + D3DXVECTOR2(-32, 32)));
		parent_->AddTask(new Player_DeadInvertTask(parent_, 900, DU_INV - WT_2, posInvert + D3DXVECTOR2(32, 32)));
		break;
	case WT_1 + WT_2 + WT_3:
		parent_->AddTask(new Player_DeadInvertTask(parent_, 800, DU_INV - WT_2 - WT_3, posInvert));
		frameEnd_ = 0;
		break;
	}

	++frame_;
}

//*******************************************************************
//Player_DeadCircleTask
//*******************************************************************
Player_DeadCircleTask::Player_DeadCircleTask(Scene* parent, CD3DXVECTOR2 pos) : TaskBase(parent) {
	frameEnd_ = 50;

	{
		GET_INSTANCE(ResourceManager, resourceManager);

		auto textureCircle = resourceManager->GetResourceAs<TextureResource>("img/player/eff_dead.png");

		objCircle_.SetTexture(textureCircle);
		objCircle_.SetSourceRectNormalized(DxRectangle<float>(0, 0, 1, 1));
		objCircle_.SetDestCenter();
		objCircle_.UpdateVertexBuffer();
	}

	objCircle_.SetBlendType(BlendMode::Add);
	objCircle_.SetPosition(pos);
}
void Player_DeadCircleTask::Render(byte layer) {
	if (layer != LAYER_EX_UI - 2) return;
	objCircle_.Render();
}
void Player_DeadCircleTask::Update() {
	static double tAngle[2];
	if (frame_ == 0) {
		GET_INSTANCE(RandProvider, rand);
		tAngle[0] = rand->GetReal(0, GM_PI_X2);
		tAngle[1] = GM_DTORA(rand->GetReal(2.8, 4.5)) * SystemUtility::RandDirection();
	}

	double tmp = frame_ / (double)(frameEnd_ - 1);
	double tmp_s = Math::Lerp::Smooth(0.08, 1.6, tmp);
	int tmp_a = Math::Lerp::Linear(384, 0, tmp);

	objCircle_.SetAngleZ(tAngle[0]);
	objCircle_.SetScale(tmp_s, tmp_s, 1);
	objCircle_.SetAlpha(tmp_a);

	tAngle[0] += tAngle[1];

	++frame_;
}

//*******************************************************************
//Player_DeadInvertTask
//*******************************************************************
Player_DeadInvertTask::Player_DeadInvertTask(Scene* parent, double radius, size_t frameEnd, CD3DXVECTOR2 pos) : TaskBase(parent) {
	frameEnd_ = frameEnd;

	{
		constexpr size_t countEdge = 64;

		std::vector<VertexTLX> vertex;
		vertex.resize(countEdge + 1);

		std::vector<uint16_t> index;
		index.resize((countEdge + 1) * 2);

		vertex[countEdge] = VertexTLX(D3DXVECTOR3(0, 0, 0));
		for (size_t i = 0; i < countEdge; ++i) {
			double va = GM_PI_X2 / countEdge * i;
			vertex[i] = VertexTLX(D3DXVECTOR3(radius * cos(va), radius * sin(va), 1));

			index[i * 2 + 0] = i;
			index[i * 2 + 1] = countEdge;
		}
		index[countEdge * 2 + 0] = 0;
		index[countEdge * 2 + 1] = countEdge;

		objCircle_.SetArrayVertex(vertex);
		objCircle_.SetArrayIndex(index);
	}

	objCircle_.SetPrimitiveType(D3DPT_TRIANGLESTRIP);
	objCircle_.SetBlendType(BlendMode::Invert);
	objCircle_.SetPosition(pos);
}
void Player_DeadInvertTask::Render(byte layer) {
	if (layer != LAYER_EX_UI - 1) return;
	objCircle_.Render();
}
void Player_DeadInvertTask::Update() {
	double tmp = frame_ / ((double)frameEnd_ - 1);
	double tmp_s = Math::Lerp::Accelerate<double>(0, 1, tmp);

	objCircle_.SetScale(tmp_s, tmp_s, 1);

	++frame_;
}

//*******************************************************************
//Player_DeadParticleTask
//*******************************************************************
Player_DeadParticleTask::Player_DeadParticleTask(Scene* parent, CD3DXVECTOR2 pos) : TaskBase(parent) {
	GET_INSTANCE(ResourceManager, resourceManager);
	GET_INSTANCE(RandProvider, rand);

	auto textureParticle = resourceManager->GetResourceAs<TextureResource>("img/player/eff_base.png");

	size_t countParticle = rand->GetInt(32, 48);
	for (size_t i = 0; i < countParticle; ++i) {
		Particle newParticle;

		newParticle.obj.SetTexture(textureParticle);
		newParticle.obj.SetSourceRect(DxRectangle<float>::SetFromIndex(16, 16, rand->GetInt() % 4, 4));
		newParticle.obj.SetDestCenter();
		newParticle.obj.UpdateVertexBuffer();

		newParticle.obj.SetBlendType(BlendMode::Subtract);
		newParticle.obj.SetColor(rand->GetInt(48, 255), 16, rand->GetInt(48, 255));

		newParticle.pos = pos + D3DXVECTOR2(rand->GetReal(-4, 4), rand->GetReal(-4, 4));
		{
			double angle = GM_PI_X2 / countParticle * (i + rand->GetReal(-0.35, 0.35));
			double speed = rand->GetReal(3.2, 9.6);
			newParticle.move = D3DXVECTOR2(speed * cos(angle), speed * sin(angle));
		}
		newParticle.scale = rand->GetReal(3, 7);
		newParticle.alpha = 255;
		newParticle.life = rand->GetInt(40, 90);
		newParticle.frame = 0;

		listParticle_.push_back(newParticle);
	}
}
void Player_DeadParticleTask::Render(byte layer) {
	if (layer != LAYER_EX_UI - 3) return;
	for (auto& iParticle : listParticle_) {
		if (iParticle.frame < iParticle.life)
			iParticle.obj.Render();
	}
}
void Player_DeadParticleTask::Update() {
	bool bAnyActive = false;

	for (auto& iParticle : listParticle_) {
		if (iParticle.frame < iParticle.life) {
			bAnyActive = true;

			double rate = frame_ / (double)(iParticle.life - 1);
			double tScale = (1 - rate) * pow(iParticle.scale, 0.7);
			double tAlpha = abs(sin(rate * GM_PI)) * 255;

			iParticle.obj.SetPosition(iParticle.pos);
			iParticle.obj.SetScale(tScale, tScale, 1);
			iParticle.obj.SetAlpha(tAlpha * 255);

			iParticle.pos += iParticle.move;
			iParticle.move *= 0.99f;

			++(iParticle.frame);
		}
	}

	if (!bAnyActive) frameEnd_ = 0;
	++frame_;
}