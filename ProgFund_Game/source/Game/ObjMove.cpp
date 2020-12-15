#include "pch.h"

#include "ObjMove.hpp"

//*******************************************************************
//Stage_ObjMove
//*******************************************************************
Stage_ObjMove::Stage_ObjMove() {
	posX_ = 0;
	posY_ = 0;

	frameObjMove_ = 0;
	bEnableMove_ = true;
}

void Stage_ObjMove::_Move() {
	if (pattern_ == nullptr || !bEnableMove_) return;

	if (mapWaitPattern_.size() > 0) {
		auto itrFirst = mapWaitPattern_.begin();
		while (frameObjMove_ >= itrFirst->first) {
			if (pattern_ != nullptr)
				itrFirst->second->_CopyFromPrevious(pattern_.get());
			pattern_ = itrFirst->second;

			itrFirst = mapWaitPattern_.erase(itrFirst);
			if (mapWaitPattern_.size() == 0) break;
		}
	}

	pattern_->Move();
	++frameObjMove_;
}

double Stage_ObjMove::GetSpeed() {
	return pattern_ == nullptr ? 0 : pattern_->GetSpeed();
}
void Stage_ObjMove::SetSpeed(double speed) {
	if (pattern_ == nullptr || pattern_->GetPatternType() != Stage_MovePattern::Type::Angle)
		pattern_ = shared_ptr<Stage_MovePatternAngle>(new Stage_MovePatternAngle(this));
	((Stage_MovePatternAngle*)pattern_.get())->SetSpeed(speed);
}
double Stage_ObjMove::GetDirectionAngle() {
	return pattern_ == nullptr ? 0 : pattern_->GetDirectionAngle();
}
void Stage_ObjMove::SetDirectionAngle(double angle) {
	if (pattern_ == nullptr || pattern_->GetPatternType() != Stage_MovePattern::Type::Angle)
		pattern_ = shared_ptr<Stage_MovePatternAngle>(new Stage_MovePatternAngle(this));
	((Stage_MovePatternAngle*)pattern_.get())->SetDirectionAngle(angle);
}

void Stage_ObjMove::SetSpeedX(double speedX) {
	if (pattern_ == nullptr || pattern_->GetPatternType() != Stage_MovePattern::Type::XY)
		pattern_.reset(new Stage_MovePatternXY(this));
	((Stage_MovePatternXY*)pattern_.get())->c_ = speedX;
}
void Stage_ObjMove::SetSpeedY(double speedY) {
	if (pattern_ == nullptr || pattern_->GetPatternType() != Stage_MovePattern::Type::XY)
		pattern_.reset(new Stage_MovePatternXY(this));
	((Stage_MovePatternXY*)pattern_.get())->s_ = speedY;
}

//*******************************************************************
//Stage_MovePattern
//*******************************************************************
Stage_MovePattern::Stage_MovePattern(Stage_ObjMove* target) {
	moveTarget_ = target;

	typePattern_ = Type::Unknown;
	framePattern_ = 0;

	c_ = 1;
	s_ = 0;
}

//*******************************************************************
//Stage_MovePatternAngle
//*******************************************************************
Stage_MovePatternAngle::Stage_MovePatternAngle(Stage_ObjMove* target) : Stage_MovePattern(target) {
	typePattern_ = Type::Angle;

	speed_ = 0;
	angDirection_ = 0;
	acceleration_ = 0;
	maxSpeed_ = 0;
	angularVelocity_ = 0;
}

#define RETAIN_VAL(a, b) if (a == Stage_MovePattern::NO_CHANGE) a = b;

void Stage_MovePatternAngle::_CopyFromPrevious(Stage_MovePattern* prev) {
	double speed = 0;
	double angle = 0;
	double accel = 0;
	double maxsp = 0;
	double angvl = 0;

	if (prev->GetPatternType() == Stage_MovePattern::Type::Angle) {
		Stage_MovePatternAngle* nSrc = (Stage_MovePatternAngle*)prev;
		speed = nSrc->speed_;
		angle = nSrc->angDirection_;
		accel = nSrc->acceleration_;
		maxsp = nSrc->angularVelocity_;
		angvl = nSrc->maxSpeed_;
	}
	else if (prev->GetPatternType() == Stage_MovePattern::Type::XY) {
		Stage_MovePatternXY* nSrc = (Stage_MovePatternXY*)prev;
		speed = nSrc->GetSpeed();
		angle = nSrc->GetDirectionAngle();
		accel = hypot(nSrc->accelerationX_, nSrc->accelerationY_);
		maxsp = hypot(nSrc->maxSpeedX_, nSrc->maxSpeedY_);
		angvl = 0;
	}

	RETAIN_VAL(speed_, speed);
	RETAIN_VAL(angDirection_, angle);
	RETAIN_VAL(acceleration_, accel);
	RETAIN_VAL(maxSpeed_, maxsp);
	RETAIN_VAL(angularVelocity_, angvl);

	SetDirectionAngle(angDirection_, true);
}
void Stage_MovePatternAngle::Move() {
	if (acceleration_ != 0) {
		speed_ += acceleration_;
		if (acceleration_ > 0)
			speed_ = std::min(speed_, maxSpeed_);
		if (acceleration_ < 0)
			speed_ = std::max(speed_, maxSpeed_);
	}
	if (angularVelocity_ != 0) {
		SetDirectionAngle(angDirection_ + angularVelocity_);
	}

	moveTarget_->posX_ += GetSpeedX();
	moveTarget_->posY_ += GetSpeedY();

	++framePattern_;
}

void Stage_MovePatternAngle::SetDirectionAngle(double angle, bool bReset) {
	if (bReset || angle != angDirection_) {
		c_ = cos(angle);
		s_ = sin(angle);
	}
	angDirection_ = Math::NormalizeAngleRad(angle);
}

//*******************************************************************
//Stage_MovePatternXY
//*******************************************************************
Stage_MovePatternXY::Stage_MovePatternXY(Stage_ObjMove* target) : Stage_MovePattern(target) {
	typePattern_ = Type::XY;

	c_ = 0;
	s_ = 0;
	accelerationX_ = 0;
	accelerationY_ = 0;
	maxSpeedX_ = 0;
	maxSpeedY_ = 0;
}

void Stage_MovePatternXY::_CopyFromPrevious(Stage_MovePattern* prev) {
	double sx = 0, sy = 0;
	double ax = 0, ay = 0;
	double mx = 0, my = 0;

	if (prev->GetPatternType() == Stage_MovePattern::Type::Angle) {
		Stage_MovePatternAngle* nSrc = (Stage_MovePatternAngle*)prev;
		sx = nSrc->GetSpeedX();
		sy = nSrc->GetSpeedY();
		ax = nSrc->acceleration_ * c_;
		ay = nSrc->acceleration_ * s_;
		mx = nSrc->maxSpeed_ * c_;
		my = nSrc->maxSpeed_ * s_;
	}
	else if (prev->GetPatternType() == Stage_MovePattern::Type::XY) {
		Stage_MovePatternXY* nSrc = (Stage_MovePatternXY*)prev;
		sx = nSrc->c_;
		sy = nSrc->s_;
		ax = nSrc->accelerationX_;
		ay = nSrc->accelerationY_;
		mx = nSrc->maxSpeedX_;
		my = nSrc->maxSpeedY_;
	}

	RETAIN_VAL(c_, sx);
	RETAIN_VAL(s_, sy);
	RETAIN_VAL(accelerationX_, ax);
	RETAIN_VAL(accelerationY_, ay);
	RETAIN_VAL(maxSpeedX_, mx);
	RETAIN_VAL(maxSpeedY_, my);
}
void Stage_MovePatternXY::Move() {
	if (accelerationX_ != 0) {
		c_ += accelerationX_;
		if (accelerationX_ > 0)
			c_ = std::min(c_, maxSpeedX_);
		if (accelerationX_ < 0)
			c_ = std::max(c_, maxSpeedX_);
	}
	if (accelerationY_ != 0) {
		s_ += accelerationY_;
		if (accelerationY_ > 0)
			s_ = std::min(s_, maxSpeedY_);
		if (accelerationY_ < 0)
			s_ = std::max(s_, maxSpeedY_);
	}

	moveTarget_->posX_ += c_;
	moveTarget_->posY_ += s_;

	++framePattern_;
}

//*******************************************************************
//Stage_MovePatternLine
//*******************************************************************
Stage_MovePatternLine::Stage_MovePatternLine(Stage_ObjMove* target) : Stage_MovePattern(target) {
	typePattern_ = Type::Line;

	typeLine_ = TypeLine::None;
	frameLineMax_ = 1;

	speed_ = 0;
	angDirection_ = 0;

	ZeroMemory(posIni_, sizeof(posIni_));
	ZeroMemory(posTarget_, sizeof(posTarget_));
}
void Stage_MovePatternLine::Move() {
	if (framePattern_ < frameLineMax_) {
		moveTarget_->posX_ += c_ * speed_;
		moveTarget_->posY_ += s_ * speed_;
	}
	else {
		speed_ = 0;
	}

	++framePattern_;
}

//*******************************************************************
//Stage_MovePatternLine_Speed
//*******************************************************************
Stage_MovePatternLine_Speed::Stage_MovePatternLine_Speed(Stage_ObjMove* target) : Stage_MovePatternLine(target) {
	typeLine_ = TypeLine::Speed;
}

void Stage_MovePatternLine_Speed::SetAtSpeed(double tx, double ty, double speed) {
	posIni_[0] = moveTarget_->posX_;
	posIni_[1] = moveTarget_->posY_;
	posTarget_[0] = tx;
	posTarget_[1] = ty;

	double dx = posTarget_[0] - posIni_[0];
	double dy = posTarget_[1] - posIni_[1];
	double dist = hypot(dx, dy);

	angDirection_ = atan2(dy, dx);
	frameLineMax_ = std::trunc(dist / speed + 0.001);
	speed_ = dist / frameLineMax_;	//Speed correction to reach the destination in integer frames

	c_ = dx / dist;
	s_ = dy / dist;
}

//*******************************************************************
//Stage_MovePatternLine_Frame
//*******************************************************************
Stage_MovePatternLine_Frame::Stage_MovePatternLine_Frame(Stage_ObjMove* target) : Stage_MovePatternLine(target) {
	typeLine_ = TypeLine::Frame;

	speedRateBase_ = 0;

	ZeroMemory(posDiff_, sizeof(posDiff_));
}

void Stage_MovePatternLine_Frame::SetAtFrame(double tx, double ty, size_t frame, Math::Lerp::Type typeLerp) {
	lerp_func funcLerp;
	lerp_diff_func funcLerpDiff;
	switch (typeLerp) {
	case Math::Lerp::MODE_SMOOTH:
		funcLerp = Math::Lerp::Smooth<double, double>;
		funcLerpDiff = Math::Lerp::DifferentiateSmooth<double>;
		break;
	case Math::Lerp::MODE_SMOOTHER:
		funcLerp = Math::Lerp::Smoother<double, double>;
		funcLerpDiff = Math::Lerp::DifferentiateSmoother<double>;
		break;
	case Math::Lerp::MODE_ACCELERATE:
		funcLerp = Math::Lerp::Accelerate<double, double>;
		funcLerpDiff = Math::Lerp::DifferentiateAccelerate<double>;
		break;
	case Math::Lerp::MODE_DECELERATE:
		funcLerp = Math::Lerp::Decelerate<double, double>;
		funcLerpDiff = Math::Lerp::DifferentiateDecelerate<double>;
		break;
	case Math::Lerp::MODE_LINEAR:
	default:
		funcLerp = Math::Lerp::Linear<double, double>;
		funcLerpDiff = Math::Lerp::DifferentiateLinear<double>;
		break;
	}
	SetAtFrame(tx, ty, frame, funcLerp, funcLerpDiff);
}
void Stage_MovePatternLine_Frame::SetAtFrame(double tx, double ty, size_t frame, lerp_func lerpFunc, lerp_diff_func lerpDiffFunc) {
	posIni_[0] = moveTarget_->posX_;
	posIni_[1] = moveTarget_->posY_;
	posTarget_[0] = tx;
	posTarget_[1] = ty;

	funcMove_ = lerpFunc;
	funcSpeedDiff_ = lerpDiffFunc;

	frameLineMax_ = std::max(1U, frame);

	posDiff_[0] = posTarget_[0] - posIni_[0];
	posDiff_[1] = posTarget_[1] - posIni_[1];

	angDirection_ = atan2(posDiff_[1], posDiff_[0]);

	double dist = hypot(posDiff_[0], posDiff_[1]);
	speedRateBase_ = dist / frameLineMax_;

	speed_ = speedRateBase_ * funcSpeedDiff_(0);
	
	c_ = posDiff_[0] / dist;
	s_ = posDiff_[1] / dist;
}

void Stage_MovePatternLine_Frame::Move() {
	if (framePattern_ < frameLineMax_) {
		double rLine = frameLineMax_ > 1U ? (framePattern_ / (double)(frameLineMax_ - 1)) : 1.0;

		moveTarget_->posX_ = funcMove_(posIni_[0], posTarget_[0], rLine);
		moveTarget_->posY_ = funcMove_(posIni_[1], posTarget_[1], rLine);

		speed_ = speedRateBase_ * funcSpeedDiff_(rLine);
	}
	else {
		speed_ = 0;
	}

	++framePattern_;
}