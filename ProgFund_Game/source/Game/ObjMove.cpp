#include "pch.h"

#include "ObjMove.hpp"

//*******************************************************************
//Stage_ObjMove
//*******************************************************************
Stage_ObjMove::Stage_ObjMove() {
	posX_ = 0;
	posY_ = 0;
	bEnableMove_ = true;
}

void Stage_ObjMove::_Move() {
	if (pattern_ == nullptr || !bEnableMove_) return;

	pattern_->Move();
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

void Stage_MovePatternAngle::_Activate(Stage_MovePattern* src) {
	if (src->GetPatternType() == Stage_MovePattern::Type::Angle) {
		Stage_MovePatternAngle* nSrc = (Stage_MovePatternAngle*)src;
		speed_ = nSrc->speed_;
		angDirection_ = nSrc->angDirection_;
		acceleration_ = nSrc->acceleration_;
		maxSpeed_ = nSrc->angularVelocity_;
		angularVelocity_ = nSrc->maxSpeed_;
	}
	else if (src->GetPatternType() == Stage_MovePattern::Type::XY) {
		Stage_MovePatternXY* nSrc = (Stage_MovePatternXY*)src;
		speed_ = nSrc->GetSpeed();
		angDirection_ = nSrc->GetDirectionAngle();
		acceleration_ = hypot(nSrc->accelerationX_, nSrc->accelerationY_);
		maxSpeed_ = hypot(nSrc->maxSpeedX_, nSrc->maxSpeedY_);
		angularVelocity_ = 0;
	}
	SetDirectionAngle(angDirection_);
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

void Stage_MovePatternAngle::SetDirectionAngle(double angle) {
	if (angle != angDirection_) {
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

	accelerationX_ = 0;
	accelerationY_ = 0;
	maxSpeedX_ = 0;
	maxSpeedY_ = 0;
}

void Stage_MovePatternXY::_Activate(Stage_MovePattern* src) {
	if (src->GetPatternType() == Stage_MovePattern::Type::Angle) {
		Stage_MovePatternAngle* nSrc = (Stage_MovePatternAngle*)src;
		c_ = nSrc->GetSpeedX();
		s_ = nSrc->GetSpeedY();
		accelerationX_ = nSrc->acceleration_ * c_;
		accelerationY_ = nSrc->acceleration_ * s_;
		maxSpeedX_ = nSrc->maxSpeed_ * c_;
		maxSpeedY_ = nSrc->maxSpeed_ * s_;
	}
	else if (src->GetPatternType() == Stage_MovePattern::Type::XY) {
		Stage_MovePatternXY* nSrc = (Stage_MovePatternXY*)src;
		c_ = nSrc->c_;
		s_ = nSrc->s_;
		accelerationX_ = nSrc->accelerationX_;
		accelerationY_ = nSrc->accelerationY_;
		maxSpeedX_ = nSrc->maxSpeedX_;
		maxSpeedY_ = nSrc->maxSpeedY_;
	}
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