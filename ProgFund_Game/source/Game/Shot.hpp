#pragma once
#include "../../pch.h"

#include "GConstant.hpp"

#include "System.hpp"

class Stage_ShotAnimation;
class Stage_ObjShot;

class Stage_MainScene;
class Stage_ShotManager : public TaskBase {
public:
	enum {
		SHOT_MAX = 8192,
	};
private:
	std::list<shared_ptr<Stage_ObjShot>> listShotEnemy_;
	std::list<shared_ptr<Stage_ObjShot>> listShotPlayer_;

	std::map<int, Stage_ShotAnimation*> mapShotDataEnemy_;
	std::map<int, Stage_ShotAnimation*> mapShotDataPlayer_;
public:
	Stage_ShotManager(Scene* parent);
	~Stage_ShotManager();

	virtual void Render();
	virtual void Update();

	void LoadEnemyShotData();
	void LoadPlayerShotData();

	void AddEnemyShot(shared_ptr<Stage_ObjShot> obj);
	void AddPlayerShot(shared_ptr<Stage_ObjShot> obj);
	void CreateShotA1(CD3DXVECTOR2 pos, double speed, double angle, int graphic, size_t delay);

	void DeleteInCircle(Stage_ObjShot::Owner typeOwner, int cx, int cy, int radius);

	size_t GetEnemyShotCount() { return listShotEnemy_.size(); }
	size_t GetPlayerShotCount() { return listShotPlayer_.size(); }
};

class Stage_ShotAnimation {
	friend class Stage_ShotManager;
public:
	struct Frame {
		DxRect<float> rcSrc;
		DxRect<float> rcDst;
	};
private:
	shared_ptr<TextureResource> texture_;

	std::vector<Frame> listFrameData_;

	double spin_;
	bool bFixedAngle_;
public:
	Stage_ShotAnimation(shared_ptr<TextureResource> texture);
	~Stage_ShotAnimation();

	const Frame* GetFrame(size_t frame);
	size_t GetFrameCount() { return listFrameData_.size(); }
};

class Stage_ObjShot : public TaskBase {
	friend class Stage_ShotManager;
public:
	enum class Polarity : byte {
		None,
		White,
		Black,
	};
	enum class Owner : byte {
		Player,
		Enemy,
	};
protected:
	Polarity polarity_;
	Owner typeOwner_;

	size_t frameDelay_;
	size_t frameClipImmune_;	//The shot will not be auto-deleted when going off the screen if this is > 0
	size_t frameFadeDelete_;	//The shot will be deleted once this reaches 0

	bool bIntersectionEnable_;

	double speedX_;
	double speedY_;

	virtual void _DeleteInAutoClip();
	virtual void _CommonUpdate();
public:
	Stage_ObjShot(Scene* parent);
	~Stage_ObjShot();

	virtual void Render();
	virtual void Update();

	Polarity GetPolarity() { return polarity_; }
	Owner GetOwnerType() { return typeOwner_; }

	bool CanIntersect() { return bIntersectionEnable_; }
	//virtual void Intersect(shared_ptr<Stage_Intersectable> ownRef, shared_ptr<Stage_Intersectable> otherRef);
};