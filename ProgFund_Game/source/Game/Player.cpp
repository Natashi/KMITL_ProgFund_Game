#include "pch.h"
#include "Player.hpp"

#include "Shot.hpp"
#include "StageMain.hpp"

//*******************************************************************
//Stage_PlayerTask
//*******************************************************************
CONSTRUCT_TASK(Stage_PlayerTask) {
	moveAngle_ = 0;
	moveSpeed_ = 0;
	moveSpeedX_ = 0;
	moveSpeedY_ = 0;

	playerPos_ = D3DXVECTOR2(0, 0);
	rcClip_ = DxRectangle<int>(0, 0, 640, 480);

	frameAnimLeft_ = 0;
	frameAnimRight_ = 0;

	bFocus_ = false;

	{
		GET_INSTANCE(ResourceManager, resourceManager);

		auto texturePlayer = resourceManager->GetResourceAs<TextureResource>("img/player/pl00.png");

		objSprite_.SetTexture(texturePlayer);
		objSprite_.SetSourceRect(DxRectangle(0, 0, 32, 48));
		objSprite_.SetDestCenter();
		objSprite_.UpdateVertexBuffer();
	}
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

			moveSpeed_ = hypot(sx, sy);
			if (moveSpeed_ != 0)
				moveAngle_ = hypot(sy, sx);
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

			moveSpeed_ = speed;
			if (sx != 0 && sy != 0) {	//Normalize axes
				constexpr double diagFactor = 1.0 / GM_SQRT2;
				sx *= diagFactor;
				sy *= diagFactor;

				if (sx > 0)
					moveAngle_ = sy > 0 ? Math::DegreeToRadian(45) : Math::DegreeToRadian(315);
				else
					moveAngle_ = sy > 0 ? Math::DegreeToRadian(135) : Math::DegreeToRadian(225);
			}
			else if (sx != 0 || sy != 0) {
				if (sx != 0)
					moveAngle_ = sx > 0 ? Math::DegreeToRadian(0) : Math::DegreeToRadian(180);
				else if (sy != 0)
					moveAngle_ = sy > 0 ? Math::DegreeToRadian(90) : Math::DegreeToRadian(270);
			}
			else {
				moveSpeed_ = 0;
			}
		}
	}

	moveSpeedX_ = sx;
	moveSpeedY_ = sy;

	//Add and clip player position
	{
		playerPos_.x = std::clamp<float>(playerPos_.x + sx, rcClip_.left, rcClip_.right);
		playerPos_.y = std::clamp<float>(playerPos_.y + sy, rcClip_.top, rcClip_.bottom);
	}
}

void Stage_PlayerTask::Render(byte layer) {
	if (layer != LAYER_PLAYER) return;
	objSprite_.Render();
}
void Stage_PlayerTask::Update() {
	GET_INSTANCE(InputManager, inputManager);

	KeyState stateShot = inputManager->GetKeyState(VirtualKey::Shot);
	KeyState stateSlow = inputManager->GetKeyState(VirtualKey::Focus);

	bFocus_ = stateSlow == KeyState::Push || stateSlow == KeyState::Hold;

	//Update movement
	{
		_Move();
		objSprite_.SetPosition(roundf(playerPos_.x), roundf(playerPos_.y), 1.0f);

		//Update player animation
		{
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

			if (moveSpeedX_ < 0) {
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

			if (moveSpeedX_ > 0) {
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
		}
	}

	//Shoot
	{
		static size_t sFrame = 0;

		if (stateShot == KeyState::Push || stateShot == KeyState::Hold) {
			auto shotManager = ((Stage_MainScene*)parent_)->GetShotManager();

			auto AddShot = [&](int index, D3DXVECTOR2 pos, float sp, double ang, int gr) {
				if (index < 0) pos += playerPos_;
				else {
					if (taskOption_[index] == nullptr) return;
					pos += taskOption_[index]->GetPosition();
				}

				auto shot = shotManager->CreateShotA1(ShotOwnerType::Player,
					pos, sp, -GM_PI_2 + ang, gr, 0);
				shot->SetAngleZOff(-GM_PI_2);
				shotManager->AddPlayerShot(shot, ShotPolarity::White);
			};

			if (frame_ % 6 == 0) {
				AddShot(-1, D3DXVECTOR2(10, 32), 48, 0, ShotPlayerConst::Main);
				AddShot(-1, D3DXVECTOR2(-10, 32), 48, 0, ShotPlayerConst::Main);
			}

			if (!bFocus_) {
				if (frame_ % 8) goto lab_shot_exit;
				double angOff = Math::DegreeToRadian(8 + sFrame * 8);
				AddShot(0, D3DXVECTOR2(0, 0), 5, -angOff, ShotPlayerConst::Homing);
				AddShot(1, D3DXVECTOR2(0, 0), 5, angOff, ShotPlayerConst::Homing);
				sFrame = (sFrame + 1) % 5;
			}
			else {
				if (frame_ % 5) goto lab_shot_exit;
				AddShot(0, D3DXVECTOR2(-8, 4), 28, 0, ShotPlayerConst::Needle);
				AddShot(0, D3DXVECTOR2(8, 4), 28, 0, ShotPlayerConst::Needle);

				AddShot(1, D3DXVECTOR2(-8, 4), 28, 0, ShotPlayerConst::Needle);
				AddShot(1, D3DXVECTOR2(8, 4), 28, 0, ShotPlayerConst::Needle);
			}
lab_shot_exit:;
		}
	}

	{
		if (bFocus_) {
			if (*taskHitbox_ == nullptr) {
				taskHitbox_[0] = shared_ptr<Stage_PlayerHitboxTask>(
					new Stage_PlayerHitboxTask(parent_, 255, 160, 0.3, 12, -180, 180, 3));
				taskHitbox_[1] = shared_ptr<Stage_PlayerHitboxTask>(
					new Stage_PlayerHitboxTask(parent_, 0, 255, 1.2, 18, 0, 0, -2));

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
	}
	{
		if (*taskOption_ == nullptr) {
			taskOption_[0] = shared_ptr<Stage_PlayerOptionTask>(
				new Stage_PlayerOptionTask(parent_, D3DXVECTOR2(-36, 36), D3DXVECTOR2(-15, -32), 1));
			taskOption_[1] = shared_ptr<Stage_PlayerOptionTask>(
				new Stage_PlayerOptionTask(parent_, D3DXVECTOR2(36, 36), D3DXVECTOR2(15, -32), -1));

			parent_->AddTask(taskOption_[0]);
			parent_->AddTask(taskOption_[1]);
		}
	}

	++frame_;
}

//*******************************************************************
//Stage_PlayerHitboxTask
//*******************************************************************
Stage_PlayerHitboxTask::Stage_PlayerHitboxTask(Scene* parent, int s_alpha, int e_alpha, double s_scale,
	size_t s_timer, double s_angle, double e_angle, double r_spin) : TaskBase(parent)
{
	frameSub_ = 0;
	bEnd_ = false;

	iniAlpha_ = s_alpha;
	endAlpha_ = e_alpha;
	iniScale_ = s_scale;
	scaleTimer_ = s_timer;
	iniAngle1_ = s_angle;
	iniAngle2_ = e_angle;
	spinRate_ = r_spin;

	tScale_ = iniScale_;
	tAngle_ = iniAngle1_;

	{
		GET_INSTANCE(ResourceManager, resourceManager);

		auto textureHitbox = resourceManager->GetResourceAs<TextureResource>("img/player/eff_sloweffect.png");

		objHitbox_.SetTexture(textureHitbox);
		objHitbox_.SetSourceRect(DxRectangle(0, 0, 63, 63));
		objHitbox_.SetDestCenter();
		objHitbox_.UpdateVertexBuffer();
	}

	objHitbox_.SetAlpha(iniAlpha_);
	objHitbox_.SetScale(tScale_, tScale_, 1.0f);
	objHitbox_.SetAngleZ(Math::DegreeToRadian(tAngle_));
}

void Stage_PlayerHitboxTask::Render(byte layer) {
	if (layer != LAYER_SHOT + 1) return;
	objHitbox_.Render();
}
void Stage_PlayerHitboxTask::Update() {
	if (!bEnd_) {
		if (frame_ < 18) {
			tScale_ = Math::Lerp::Smooth<double>(iniScale_, 1, std::min(frame_, scaleTimer_) / (double)scaleTimer_);
			tAngle_ = Math::Lerp::Linear(iniAngle1_, iniAngle2_, frame_ / 17.0);

			objHitbox_.SetAlpha(Math::Lerp::Linear(iniAlpha_, endAlpha_, std::min(frame_, 6U) / 6.0));
		}
		else {
			if (frame_ == 18) objHitbox_.SetAlpha(endAlpha_);
			tAngle_ += spinRate_;
		}
	}
	else {
		static double lScale;
		if (frameEnd_ == UINT_MAX) {
			frameEnd_ = 6;
			frame_ = 0;
			lScale = tScale_;
		}

		tScale_ = Math::Lerp::Accelerate<double>(lScale, 0, frame_ / 5.0);
		tAngle_ += spinRate_;
	}

	{
		auto objPlayer = ((Stage_MainScene*)parent_)->GetPlayer();
		objHitbox_.SetX(roundf(objPlayer->GetX()));
		objHitbox_.SetY(roundf(objPlayer->GetY()));
	}
	objHitbox_.SetScale(tScale_, tScale_, 1.0f);
	objHitbox_.SetAngleZ(Math::DegreeToRadian(tAngle_));

	++frame_;
}

//*******************************************************************
//Stage_PlayerOptionTask
//*******************************************************************
Stage_PlayerOptionTask::Stage_PlayerOptionTask(Scene* parent, 
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
		position_ = objPlayer->GetPosition();
	}
}

void Stage_PlayerOptionTask::Render(byte layer) {
	if (layer != LAYER_OPTION) return;
	objOption_.Render();
}
void Stage_PlayerOptionTask::Update() {
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

		position_.x = Math::Lerp::Linear(position_.x, tx, 0.3);
		position_.y = Math::Lerp::Linear(position_.y, ty, 0.3);
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