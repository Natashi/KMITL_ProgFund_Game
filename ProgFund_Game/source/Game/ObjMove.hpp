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

	size_t frameObjMove_;
	bool bEnableMove_;
protected:
	std::map<size_t, shared_ptr<Stage_MovePattern>> mapWaitPattern_;
	shared_ptr<Stage_MovePattern> pattern_;

	virtual void _Move();
public:
	Stage_ObjMove();

	double GetX() { return posX_; }
	void SetX(double x) { posX_ = x; }
	double GetY() { return posY_; }
	void SetY(double y) { posY_ = y; }
	D3DXVECTOR2 GetMovePosition() { return D3DXVECTOR2(posX_, posY_); }

	double GetSpeed();
	void SetSpeed(double speed);
	double GetDirectionAngle();
	void SetDirectionAngle(double angle);

	void SetSpeedX(double speedX);
	void SetSpeedY(double speedY);

	shared_ptr<Stage_MovePattern> GetPattern() { return pattern_; }
	void SetPattern(shared_ptr<Stage_MovePattern> pattern) { pattern_ = pattern; }
	void SetPattern(Stage_MovePattern* pattern) { pattern_.reset(pattern); }

	void AddPattern(size_t frame, shared_ptr<Stage_MovePattern> pattern) { mapWaitPattern_[frameObjMove_ + frame] = pattern; }
	void AddPattern(size_t frame, Stage_MovePattern* pattern) { AddPattern(frame, shared_ptr<Stage_MovePattern>(pattern)); }

	double GetDeltaAngle(Stage_ObjMove* other) {
		if (other == nullptr) return 0;
		return GetDeltaAngle(other->posX_, other->posY_);
	}
	double GetDeltaAngle(double x, double y) {
		return atan2(y - posY_, x - posX_);
	}
	static double GetDeltaAngle(Stage_ObjMove* lo, Stage_ObjMove* ro) {
		return lo->GetDeltaAngle(ro);
	}
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
public:
	Stage_ObjMove* moveTarget_;

	Type typePattern_;
	size_t framePattern_;

	double c_;
	double s_;
protected:
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
class Stage_MovePatternLine : public Stage_MovePattern {
public:
	enum class TypeLine : byte {
		None,
		Speed,
		Frame,
	};
public:
	TypeLine typeLine_;
	size_t frameLineMax_;

	double speed_;
	double angDirection_;

	double posIni_[2];
	double posTarget_[2];
protected:
	virtual void _Activate(Stage_MovePattern* src) {}
public:
	Stage_MovePatternLine(Stage_ObjMove* target);

	virtual void Move();

	virtual inline double GetSpeed() { return speed_; }
	virtual inline double GetDirectionAngle() { return angDirection_; }

	virtual double GetSpeedX() { return c_ * speed_; }
	virtual double GetSpeedY() { return s_ * speed_; }
};
class Stage_MovePatternLine_Speed : public Stage_MovePatternLine {
public:
	Stage_MovePatternLine_Speed(Stage_ObjMove* target);

	void SetAtSpeed(double tx, double ty, double speed);
};
class Stage_MovePatternLine_Frame : public Stage_MovePatternLine {
public:
	typedef double (*lerp_func)(double, double, double);
	typedef double (*lerp_diff_func)(double);
public:
	double speedRateBase_;
	lerp_func funcMove_;
	lerp_diff_func funcSpeedDiff_;
	double posDiff_[2];
public:
	Stage_MovePatternLine_Frame(Stage_ObjMove* target);

	void SetAtFrame(double tx, double ty, size_t frame) { SetAtFrame(tx, ty, frame, Math::Lerp::MODE_LINEAR); }
	void SetAtFrame(double tx, double ty, size_t frame, Math::Lerp::Type typeLerp);
	void SetAtFrame(double tx, double ty, size_t frame, lerp_func lerpFunc, lerp_diff_func lerpDiffFunc);

	virtual void Move();
};