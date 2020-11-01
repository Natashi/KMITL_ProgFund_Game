#include "pch.h"
#include "Player.hpp"

//*******************************************************************
//Stage_PlayerTask
//*******************************************************************
Stage_PlayerTask::Stage_PlayerTask(Scene* parent) : TaskBase(parent) {
	moveAngle_ = 0;
	moveSpeed_ = 0;

	playerPos_ = D3DXVECTOR2(0, 0);
	rcClip_ = DxRect<int>(0, 0, 640, 480);

	frameAnimLeft_ = 0;
	frameAnimRight_ = 0;

	{
		GET_INSTANCE(ResourceManager, resourceManager);

		auto texturePlayer = resourceManager->GetResourceAs<TextureResource>("img/player/pl00.png");

		objSprite_.SetTexture(texturePlayer);
		objSprite_.SetSourceRect(DxRect(0, 0, 32, 48));
		objSprite_.SetDestCenter();
		objSprite_.UpdateVertexBuffer();
	}
}
Stage_PlayerTask::~Stage_PlayerTask() {
}

void Stage_PlayerTask::Render() {
	objSprite_.Render();
}
void Stage_PlayerTask::Update() {
	GET_INSTANCE(InputManager, inputManager);

	//Update movement
	{
		KeyState stateLeft = inputManager->GetKeyState(VirtualKey::Left);
		KeyState stateRight = inputManager->GetKeyState(VirtualKey::Right);
		KeyState stateUp = inputManager->GetKeyState(VirtualKey::Up);
		KeyState stateDown = inputManager->GetKeyState(VirtualKey::Down);
		KeyState stateSlow = inputManager->GetKeyState(VirtualKey::Focus);

		double sx = 0;
		double sy = 0;
		double speed = stateSlow == KeyState::Push || stateSlow == KeyState::Hold
			? 1.75 : 4;
		//speed /= 2;

		bool bKeyLeft = stateLeft == KeyState::Push || stateLeft == KeyState::Hold;
		bool bKeyRight = stateRight == KeyState::Push || stateRight == KeyState::Hold;
		bool bKeyUp = stateUp == KeyState::Push || stateUp == KeyState::Hold;
		bool bKeyDown = stateDown == KeyState::Push || stateDown == KeyState::Hold;
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

		//Add and clip player position
		{
			playerPos_.x = std::clamp<float>(playerPos_.x + sx, rcClip_.left, rcClip_.right);
			playerPos_.y = std::clamp<float>(playerPos_.y + sy, rcClip_.top, rcClip_.bottom);
		}
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

			if (bKeyLeft) {
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

			if (bKeyRight) {
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

				DxRect<int> frameRect = DxRect<int>::SetFromIndex(32, 48, imgRender, 10);
				objSprite_.SetScroll(frameRect.left / 256.0, frameRect.top / 256.0);
				//objSprite_.SetSourceRect(frameRect);
				//objSprite_.UpdateVertexBuffer();
			}
		}
	}

	++frame_;
}