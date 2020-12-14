#pragma once
#include "../../../pch.h"

#include "../GConstant.hpp"

#include "../StageMain.hpp"

class Boss_MagicCircle;
class Enemy_LifeIndicatorBoss;
class Boss_SpellBackground;
class EnemyBoss_Shinki : public Stage_EnemyTask_Scripted {
protected:
	Sprite2D objBossAnimation_;
	int frameAnimLeft_;
	int frameAnimRight_;
	int frameAnimCharge_;
	size_t chargeTimer_;
	byte chargeMode_;

	shared_ptr<Boss_MagicCircle> pTaskCircle_;
	shared_ptr<Enemy_LifeIndicatorBoss> pTaskLifebar_;
	shared_ptr<Boss_SpellBackground> pTaskBackground_;
protected:
	void _RunAnimation();
public:
	EnemyBoss_Shinki(Scene* parent);

	virtual void Render(byte layer);
	virtual void Update();

	void SetCharging(bool b) { chargeTimer_ = b ? UINT_MAX : 0; }
	void SetChargeDuration(size_t d) { chargeTimer_ = d; }
	void SetChargeType(byte t) { chargeMode_ = t; frameAnimCharge_ = 0; }

	void SetSpellBackground(bool b);
};

class Boss_MagicCircle : public TaskBase {
	friend class EnemyBoss_Shinki;
private:
	EnemyBoss_Shinki* objBoss_;

	Sprite2D objCircle_;

	bool bEnd_;

	double tAngle_[2];
	double tScale_[2];
	double tAlpha_[2];
public:
	Boss_MagicCircle(Scene* parent, EnemyBoss_Shinki* objBoss);

	virtual void Render(byte layer);
	virtual void Update();
};
class Enemy_LifeIndicatorBoss : public TaskBase {
	friend class EnemyBoss_Shinki;
private:
	static constexpr const float BAR_WD = 4;
private:
	Stage_EnemyTask_Scripted* objEnemy_;

	StaticRenderObject2D objBar_;

	double prevScale_;
public:
	Enemy_LifeIndicatorBoss(Scene* parent, Stage_EnemyTask_Scripted* objEnemy);

	virtual void Render(byte layer);
	virtual void Update();
};

//----------------------------------------------------------------------------------

class Boss_SpellBackground : public TaskBase {
	friend class EnemyBoss_Shinki;
private:
	Sprite2D objBack_;
	Sprite2D objFront_;
	bool bVisible_;

	double tAlpha_;
public:
	Boss_SpellBackground(Scene* parent);

	virtual void Render(byte layer);
	virtual void Update();

	void SetVisible(bool b) { bVisible_ = b; }
};