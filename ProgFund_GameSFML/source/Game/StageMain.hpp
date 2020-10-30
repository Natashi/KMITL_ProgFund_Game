#pragma once
#include "../../pch.h"

#include "GConstant.hpp"

#include "System.hpp"

class Stage_MainScene : public Scene {
public:
	Stage_MainScene(SceneManager* manager);
	~Stage_MainScene();

	virtual void Render();
	virtual void Update();
private:
	shared_ptr<SoundResource> musicBackground_;
	Sprite2D objBackground_;
};