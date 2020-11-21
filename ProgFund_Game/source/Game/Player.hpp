#pragma once
#include "../../pch.h"

#include "System.hpp"
#include "Intersection.hpp"

class Stage_MainScene;

class Player_HitboxTask;
class Player_BarrierTask;
class Player_OptionTask;
class Stage_PlayerTask : public TaskBase, public Stage_ObjCollision {
	friend class Stage_MainScene;
public:
	static constexpr const double SPEED_FAST = 4;		//Unfocused move speed
	static constexpr const double SPEED_SLOW = 1.7;		//Focused move speed

	static constexpr const size_t IFRAME_DEATH = 220;	//I-Frames on death

	static constexpr const int HITBOX_SIZE = 4;			//Hitbox radius
	static constexpr const int BARRIER_SIZE = 32;		//Absorption barrier radius
	static constexpr const int BARRIER_SIZE_MIN = 24;	//Absorption barrier radius during post-death recovery

	static constexpr const int POLARITY_SWITCH_TIME = 20;		//Time for polarity scale to go from -POLARITY_SWITCH to 1.0
	static constexpr const double POLARITY_ABSORBING = 0.3;		//Min. polarity scale to start absorbing bullets
	static constexpr const double POLARITY_SWITCH = 0.15;		//Polarity scale becomes -POLARITY_SWITCH after switching
	static constexpr const int POLARITY_COOLDOWN_FRAME = 15;	//Switch cooldown
	static constexpr const double POLARITY_INC_STEP = (1 + POLARITY_SWITCH) / (double)(POLARITY_SWITCH_TIME);
	static constexpr const double POLARITY_COOLDOWN = POLARITY_INC_STEP * POLARITY_COOLDOWN_FRAME;
private:
	weak_ptr<Stage_PlayerTask> pOwnRefWeak_;

	Sprite2D objSprite_;
	int frameAnimLeft_;
	int frameAnimRight_;

	IntersectPolarity polarity_;
	int tendencyPolarity_;	//Only -1 or 1
	double scalePolarity_;

	size_t frameInvincibility_;
	bool bFocus_;

	shared_ptr<Player_HitboxTask> taskHitbox_[2];
	shared_ptr<Player_OptionTask> taskOption_[2];
	shared_ptr<Player_BarrierTask> taskBarrier_;

	D3DXVECTOR2 playerPos_;
	DxRectangle<int> rcClip_;

	double moveAngle_;
	double moveSpeed_;
	double moveSpeedX_;
	double moveSpeedY_;

	void _Move();
	void _RunAnimation();
	void _Shoot();
	void _Death();

	void _ChangePolarity();
public:
	Stage_PlayerTask(Scene* parent);
	~Stage_PlayerTask();

	virtual void Render(byte layer);
	virtual void Update();

	//-1.0 <-------- 0.0 --------> 1.0
	//Black                      White
	IntersectPolarity GetPolarity() { 
		if (scalePolarity_ >= POLARITY_ABSORBING) return IntersectPolarity::White;
		else if (scalePolarity_ <= -POLARITY_ABSORBING) return IntersectPolarity::Black;
		return IntersectPolarity::None;
	}

	bool IsInvincible() { return frameInvincibility_ > 0; }
	bool IsFocus() { return bFocus_; }

	float GetX() { return playerPos_.x; }
	void SetX(float x) { playerPos_.x = x; }
	float GetY() { return playerPos_.y; }
	void SetY(float y) { playerPos_.y = y; }
	D3DXVECTOR2& GetPosition() { return playerPos_; }

	void SetClip(const DxRectangle<int>& clip) { rcClip_ = clip; }

	virtual void Intersect(shared_ptr<Stage_IntersectionTarget> ownTarget, shared_ptr<Stage_IntersectionTarget> otherTarget);
};

class Stage_IntersectionTarget_Player : public Stage_IntersectionTarget_Circle {
	friend class Stage_IntersectionManager;
	friend class Stage_PlayerTask;
protected:
	bool bGraze_;
public:
	Stage_IntersectionTarget_Player(bool bGraze) : Stage_IntersectionTarget_Circle() { 
		type_ = TypeTarget::Player;
		bGraze_ = bGraze; 
	}

	bool IsGraze() { return bGraze_; }
};

class Player_HitboxTask : public TaskBase {
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

	double tScale[2];
	double tAngle;
public:
	Player_HitboxTask(Scene* parent, int s_alpha, int e_alpha, double s_scale,
		size_t s_timer, double s_angle, double e_angle, double r_spin);

	virtual void Render(byte layer);
	virtual void Update();
};

class Player_BarrierTask : public TaskBase {
	friend class Stage_PlayerTask;
private:
	Sprite2D objBorder_;
	Sprite2D objGlow_;

	size_t frameSub_;
	bool bEnd_;

	D3DCOLOR color_;
	double tScale[2];
	double tAlpha[4];
public:
	Player_BarrierTask(Scene* parent, D3DCOLOR color);

	virtual void Render(byte layer);
	virtual void Update();
};

class Player_OptionTask : public TaskBase {
	friend class Stage_PlayerTask;
private:
	Sprite2D objOption_;

	size_t frameSub_;
	bool bEnd_;

	D3DXVECTOR2 position_;

	D3DXVECTOR2 posUnFocus_;
	D3DXVECTOR2 posFocus_;
	double direction_;
public:
	Player_OptionTask(Scene* parent, CD3DXVECTOR2 posUF, CD3DXVECTOR2 posF, double dir);

	const D3DXVECTOR2& GetPosition() { return position_; }

	virtual void Render(byte layer);
	virtual void Update();
};

class Player_DeadManagerTask : public TaskBase {
	friend class Stage_PlayerTask;
public:
	Player_DeadManagerTask(Scene* parent);

	virtual void Update();
};
class Player_DeadCircleTask : public TaskBase {
	friend class Stage_PlayerTask;
private:
	Sprite2D objCircle_;
public:
	Player_DeadCircleTask(Scene* parent, CD3DXVECTOR2 pos);

	virtual void Render(byte layer);
	virtual void Update();
};
class Player_DeadInvertTask : public TaskBase {
	friend class Stage_PlayerTask;
private:
	StaticRenderObject2D objCircle_;
public:
	Player_DeadInvertTask(Scene* parent, double radius, size_t frameEnd, CD3DXVECTOR2 pos);

	virtual void Render(byte layer);
	virtual void Update();
};
class Player_DeadParticleTask : public TaskBase {
	friend class Stage_PlayerTask;
private:
	struct Particle {
		Sprite2D obj;
		D3DXVECTOR2 pos;
		D3DXVECTOR2 move;
		float scale;
		int alpha;
		size_t life;
		size_t frame;
	};
	std::vector<Particle> listParticle_;
public:
	Player_DeadParticleTask(Scene* parent, CD3DXVECTOR2 pos);

	virtual void Render(byte layer);
	virtual void Update();
};