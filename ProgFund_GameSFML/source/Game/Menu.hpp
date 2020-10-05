#pragma once
#include "../../pch.h"

#include "GConstant.hpp"

class Menu_Scene : public Scene {
public:
	Menu_Scene(SceneManager* manager);
	~Menu_Scene();
};

//Splash task
class Menu_SplashTask : public TaskBase {
public:
	Menu_SplashTask(Scene* parent);
	~Menu_SplashTask();

	virtual void Render();
	virtual void Update();
private:
	size_t frameOff_;
	Sprite2D objSplash_;
};

//Menu tasks
class Menu_MainTask;
class Menu_Main_Items : public TaskBase {
	friend class Menu_MainTask;
public:
	Menu_Main_Items(Scene* parent, CD3DXVECTOR2 pos, size_t index, size_t imgId);

	virtual void Render();
	virtual void Update();

	void SetSelectIndexPointer(int* ptr) { pSelectIndex_ = ptr; }
private:
	Sprite2D itemObj_;
	Sprite2D itemObjBack_;
	size_t index_;
	int* pSelectIndex_;

	float selectScale_;
};
class Menu_MainTask : public TaskBase {
public:
	Menu_MainTask(Scene* parent);
	~Menu_MainTask();

	virtual void Render();
	virtual void Update();
private:
	Sprite2D objBackground_;
	std::vector<Menu_Main_Items*> listMenuObj_;
	int selectIndex_;
};