#pragma once
#include "../../pch.h"

#include "GConstant.hpp"

#include "System.hpp"

class Stage_SceneLoader : public TaskBase {
public:
	Stage_SceneLoader(Scene* parent);
	~Stage_SceneLoader();

	virtual void Update();
};

class Stage_MainSceneUI : public Scene {
private:
	Sprite2D objFrame_;
	StaticRenderObject2D objSideText_;
	//DynamicRenderObject2D objSideTextDigits_;
public:
	Stage_MainSceneUI(SceneManager* manager);
	~Stage_MainSceneUI();

	virtual void Render();
	virtual void Update();
};

class Stage_PlayerTask;
class Stage_MainScene : public Scene {
private:
	shared_ptr<Stage_PlayerTask> pTaskPlayer_;

	DxRect<int> rcStgFrame_;
public:
	Stage_MainScene(SceneManager* manager);
	~Stage_MainScene();

	virtual void Render();
	virtual void Update();

	shared_ptr<Stage_PlayerTask> GetPlayerTask() { return pTaskPlayer_; }
};