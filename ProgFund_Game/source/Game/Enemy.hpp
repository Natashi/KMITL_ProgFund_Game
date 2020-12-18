#pragma once
#include "../../pch.h"

#include "GConstant.hpp"

#include "ObjMove.hpp"
#include "Intersection.hpp"

class Stage_MainScene;

class Stage_EnemyTask : public TaskBase, public Stage_ObjMove, public Stage_ObjCollision {
public:
	weak_ptr<Stage_EnemyTask> pOwnRefWeak_;

	IntersectPolarity polarity_;

	bool bDelete_;

	double life_;
	double rateShotDamage_;
	double rateSpellDamage_;
protected:
	virtual void _RegistIntersection();
public:
	Stage_EnemyTask(Scene* parent);
	virtual ~Stage_EnemyTask();

	virtual void Render(byte layer) {}
	virtual void Update() {}

	virtual void Intersect(Stage_IntersectionTarget* ownTarget, Stage_IntersectionTarget* otherTarget);

	void AddLife(double delta) { life_ += delta; }
};

class Stage_EnemyTask_Scripted;
class Stage_EnemyPhase : public TaskBase {
	friend class Stage_EnemyTask_Scripted;
public:
	Stage_EnemyTask_Scripted* parentEnemy_;

	size_t timerDelay_;
	int timerMax_;
	int timer_;

	double lifeMax_;
public:
	Stage_EnemyPhase(Scene* parent, Stage_EnemyTask_Scripted* objEnemy);

	virtual void Activate();

	virtual void Render(byte layer) {}
	virtual void Update();

	Stage_EnemyTask_Scripted* GetEnemy() { return parentEnemy_; }
};
class Stage_EnemyTask_Scripted : public Stage_EnemyTask {
protected:
	std::list<shared_ptr<Stage_EnemyPhase>> listPhase_;
	shared_ptr<Stage_EnemyPhase> currentPhase_;

	size_t countTotalPhase_;
protected:
	void _RunNextPhase();
public:
	Stage_EnemyTask_Scripted(Scene* parent);

	virtual void Render(byte layer) {}
	virtual void Update();

	bool IsAllPhasesFinished() { return listPhase_.size() == 0 && currentPhase_ == nullptr; }
	void InitializePhases(const std::list<shared_ptr<Stage_EnemyPhase>>& listPhase);
	shared_ptr<Stage_EnemyPhase> GetCurrentPhase() { return currentPhase_; }
};