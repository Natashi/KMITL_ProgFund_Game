#pragma once
#include "../../pch.h"

#include "GConstant.hpp"

class Menu_TaskHost : public Scene {
public:
	Menu_TaskHost(SceneManager* manager);
	~Menu_TaskHost();
};
class Menu_SplashScene : public TaskBase {
public:
	Menu_SplashScene(Scene* parent);

	virtual void Render();
	virtual void Update();
private:
	Sprite2D objSplash_;
};