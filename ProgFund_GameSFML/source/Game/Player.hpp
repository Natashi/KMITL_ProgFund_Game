#pragma once
#include "../../pch.h"

#include "System.hpp"
#include "StageMain.hpp"

class Stage_PlayerTask : public TaskBase {
private:
	Sprite2D objSprite_;
	int frameAnimLeft_;
	int frameAnimRight_;

	D3DXVECTOR2 playerPos_;
	DxRect<int> rcClip_;

	double moveAngle_;
	double moveSpeed_;
public:
	Stage_PlayerTask(Scene* parent);
	~Stage_PlayerTask();

	virtual void Render();
	virtual void Update();

	float GetX() { return playerPos_.x; }
	void SetX(float x) { playerPos_.x = x; }
	float GetY() { return playerPos_.y; }
	void SetY(float y) { playerPos_.y = y; }
	D3DXVECTOR2& GetPosition() { return playerPos_; }

	void SetClip(const DxRect<int>& clip) { rcClip_ = clip; }
};