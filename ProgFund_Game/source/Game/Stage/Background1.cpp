#include "pch.h"

#include "Background1.hpp"

//*******************************************************************
//Stage_BackgroundScene1
//*******************************************************************
Stage_BackgroundScene1::Stage_BackgroundScene1(SceneManager* manager) : Scene(manager) {
	{

	}

	bAutoDelete_ = false;
}
Stage_BackgroundScene1::~Stage_BackgroundScene1() {
}

void Stage_BackgroundScene1::Render() {
	Scene::Render();
}
void Stage_BackgroundScene1::Update() {
	Scene::Update();
}

void Stage_BackgroundScene1::AddBackground(const std::string& name, shared_ptr<TaskBase> background) {
	auto itr = AddTask(background);
	mapBackground_[name] = itr;
}
void Stage_BackgroundScene1::DeleteBackground(const std::string& name) {
	auto itr = mapBackground_.find(name);
	if (itr != mapBackground_.end()) {
		listTask_.erase(itr->second);
		mapBackground_.erase(itr);
	}
}
shared_ptr<TaskBase> Stage_BackgroundScene1::GetBackground(const std::string& name) {
	auto itr = mapBackground_.find(name);
	return itr != mapBackground_.end() ? *(itr->second) : nullptr;
}