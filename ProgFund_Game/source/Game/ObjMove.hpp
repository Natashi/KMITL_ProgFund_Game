#pragma once
#include "../../pch.h"

#include "GConstant.hpp"

class Stage_MainScene;
class Stage_MovePattern;

class Stage_ObjMove {
	friend class Stage_MovePattern;
public:
	double posX_;
	double posY_;

	bool bEnableMove_;
protected:
	shared_ptr<Stage_MovePattern> pattern_;

	virtual void _Move();
public:
	Stage_ObjMove();

	double GetX() { return posX_; }
	void SetX(double x) { posX_ = x; }
	double GetY() { return posY_; }
	void SetY(double y) { posY_ = y; }

	double GetSpeed();
	void SetSpeed(double speed);
	double GetDirectionAngle();
	void SetDirectionAngle(double angle);

	void SetSpeedX(double speedX);
	void SetSpeedY(double speedY);

	shared_ptr<Stage_MovePattern> GetPattern() { return pattern_; }
	void SetPattern(shared_ptr<Stage_MovePattern> pattern) { pattern_ = pattern; }
};

class Stage_MovePattern {
	friend class Stage_ObjMove;
public:
	enum class Type : byte {
		Unknown,
		Angle,
		XY,
		Line,
	};
protected:
	Stage_ObjMove* moveTarget_;

	Type typePattern_;
	size_t framePattern_;

	double c_;
	double s_;

	virtual void _Activate(Stage_MovePattern* src) = 0;
public:
	Stage_MovePattern(Stage_ObjMove* target);

	virtual void Move() = 0;

	Type GetPatternType() { return typePattern_; }

	virtual inline double GetSpeed() = 0;
	virtual inline double GetDirectionAngle() = 0;

	virtual double GetSpeedX() { return c_; }
	virtual double GetSpeedY() { return s_; }
};
class Stage_MovePatternAngle : public Stage_MovePattern {
public:
	double speed_;
	double angDirection_;
	double acceleration_;
	double maxSpeed_;
	double angularVelocity_;
protected:
	virtual void _Activate(Stage_MovePattern* src);
public:
	Stage_MovePatternAngle(Stage_ObjMove* target);

	virtual void Move();

	virtual inline double GetSpeed() { return speed_; }
	virtual inline double GetDirectionAngle() { return angDirection_; }

	void SetSpeed(double speed) { speed_ = speed; }
	void SetDirectionAngle(double angle);

	virtual double GetSpeedX() { return c_ * speed_; }
	virtual double GetSpeedY() { return s_ * speed_; }
};
class Stage_MovePatternXY : public Stage_MovePattern {
public:
	double accelerationX_;
	double accelerationY_;
	double maxSpeedX_;
	double maxSpeedY_;
protected:
	virtual void _Activate(Stage_MovePattern* src);
public:
	Stage_MovePatternXY(Stage_ObjMove* target);

	virtual void Move();

	virtual inline double GetSpeed() { return hypot(c_, s_); }
	virtual inline double GetDirectionAngle() { return atan2(s_, c_); }
};