#include "pch.h"

#include "Enemy.hpp"
#include "StageMain.hpp"

//*******************************************************************
//Stage_EnemyTask
//*******************************************************************
Stage_EnemyTask::Stage_EnemyTask(Scene* parent) : TaskBase(parent) {
	polarity_ = IntersectPolarity::White;

	bDelete_ = false;

	life_ = 100;
	rateShotDamage_ = 1.0;
	rateSpellDamage_ = 1.0;
}
Stage_EnemyTask::~Stage_EnemyTask() {
}

void Stage_EnemyTask::_RegistIntersection() {
	if (bDelete_) return;

	auto intersectionManager = ((Stage_MainScene*)parent_)->GetIntersectionManager();
	this->UpdateRelativeTarget(posX_, posY_);
}

void Stage_EnemyTask::Intersect(Stage_IntersectionTarget* ownTarget, Stage_IntersectionTarget* otherTarget) {
	shared_ptr<Stage_ObjCollision> pOther = otherTarget->GetParent().lock();

	switch (otherTarget->GetTargetType()) {
	case Stage_IntersectionTarget::TypeTarget::PlayerShot:
		if (auto pShot = dynamic_cast<Stage_ObjShot*>(pOther.get())) {
			life_ -= pShot->damage_ * rateShotDamage_;
		}
		break;
	case Stage_IntersectionTarget::TypeTarget::PlayerSpell:
		break;
	}
}

//*******************************************************************
//Stage_EnemyPhase
//*******************************************************************
Stage_EnemyPhase::Stage_EnemyPhase(Scene* parent, Stage_EnemyTask_Scripted* objEnemy) : TaskBase(parent) {
	parentEnemy_ = objEnemy;

	timerDelay_ = 0;
	timerMax_ = -1;
	timer_ = -1;

	lifeMax_ = 100;
}

void Stage_EnemyPhase::Activate() {
	if (parentEnemy_)
		parentEnemy_->life_ = lifeMax_;
}

void Stage_EnemyPhase::Update() {
	if (parentEnemy_)
		bFinish_ |= parentEnemy_->life_ <= 0;
	if (timerMax_ >= 0)
		bFinish_ |= timer_ <= 0;

	if (timerDelay_ > 0) --timerDelay_;
	else --timer_;

	++frame_;
}

//*******************************************************************
//Stage_EnemyTask_Scripted
//*******************************************************************
Stage_EnemyTask_Scripted::Stage_EnemyTask_Scripted(Scene* parent) : Stage_EnemyTask(parent) {
	countTotalPhase_ = 0;
}

void Stage_EnemyTask_Scripted::Update() {
	if (currentPhase_ == nullptr || currentPhase_->IsFinished()) {
		currentPhase_ = nullptr;
		_RunNextPhase();
	}
	else {
		currentPhase_->Update();
	}
}

void Stage_EnemyTask_Scripted::_RunNextPhase() {
	if (IsAllPhasesFinished()) return;

	currentPhase_ = listPhase_.front();
	listPhase_.pop_front();

	currentPhase_->Activate();
}
void Stage_EnemyTask_Scripted::InitializePhases(const std::list<shared_ptr<Stage_EnemyPhase>>& listPhase) {
	listPhase_ = listPhase;
	countTotalPhase_ = listPhase_.size();
}