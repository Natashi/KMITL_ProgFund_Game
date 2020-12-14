#pragma once
#include "../../../pch.h"

#include "../GConstant.hpp"

#include "../StageMain.hpp"

class Stage_BackgroundScene1 : public Scene {
private:
	std::map<std::string, std::list<shared_ptr<TaskBase>>::iterator> mapBackground_;
public:
	Stage_BackgroundScene1(SceneManager* manager);
	~Stage_BackgroundScene1();

	virtual void Render();
	virtual void Update();

	void AddBackground(const std::string& name, shared_ptr<TaskBase> background);
	void DeleteBackground(const std::string& name);
	shared_ptr<TaskBase> GetBackground(const std::string& name);
};