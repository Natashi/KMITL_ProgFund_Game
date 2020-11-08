#pragma once
#include "../../pch.h"

#include "System.hpp"

class Stage_MainScene;

class Stage_PlayerHitboxTask;
class Stage_PlayerTask : public TaskBase {
	static constexpr const double SPEED_FAST = 4;
	static constexpr const double SPEED_SLOW = 1.5;
private:
	Sprite2D objSprite_;
	int frameAnimLeft_;
	int frameAnimRight_;

	bool bFocus_;

	shared_ptr<Stage_PlayerHitboxTask> taskHitbox1_;
	shared_ptr<Stage_PlayerHitboxTask> taskHitbox2_;

	D3DXVECTOR2 playerPos_;
	DxRect<int> rcClip_;

	double moveAngle_;
	double moveSpeed_;
	double moveSpeedX_;
	double moveSpeedY_;

	void _Move();
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
class Stage_PlayerHitboxTask : public TaskBase {
	friend class Stage_PlayerTask;
private:
	Sprite2D objHitbox_;

	size_t frameSub_;
	bool bEnd_;

	int iniAlpha_;
	int endAlpha_;
	double iniScale_;
	size_t scaleTimer_;
	double iniAngle1_;
	double iniAngle2_;
	double spinRate_;

	double tScale_;
	double tAngle_;
public:
	Stage_PlayerHitboxTask(Scene* parent, int s_alpha, int e_alpha, double s_scale, 
		size_t s_timer, double s_angle, double e_angle, double r_spin);

	virtual void Render();
	virtual void Update();
};