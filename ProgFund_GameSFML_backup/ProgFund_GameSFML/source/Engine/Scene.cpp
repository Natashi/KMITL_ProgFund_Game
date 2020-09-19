#include "pch.h"
#include "Utility.hpp"
#include "Scene.hpp"

//*******************************************************************
//SceneManager
//*******************************************************************
SceneManager* SceneManager::base_ = nullptr;
SceneManager::SceneManager() {
}
SceneManager::~SceneManager() {
}
void SceneManager::Initialize() {
	if (base_) throw EngineError("SceneManager already initialized.");
	base_ = this;

	listScene_.resize(MAX_SCENE_SPACE);
}
void SceneManager::Render() {
	for (Scene* iScene : listScene_) {
		if (iScene)
			iScene->Render();
	}
}
void SceneManager::Update() {
	for (Scene* iScene : listScene_) {
		if (iScene)
			iScene->Update();
	}
}
void SceneManager::AddScene(Scene* ptrScene, size_t indexScene) {
	if (indexScene >= MAX_SCENE_SPACE)
		throw EngineError(StringFormat("Scene index out of bounds: %u", indexScene));
	else if (listScene_[indexScene])
		throw EngineError(StringFormat("Scene already exists: %u", indexScene));
	listScene_[indexScene] = ptrScene;
}
void SceneManager::RemoveScene(size_t indexScene) {
	ptr_delete(listScene_[indexScene]);
}
void SceneManager::RemoveScene(Scene* ptrScene) {
	for (auto iScene = listScene_.begin(); iScene != listScene_.end(); ++iScene) {
		if (*iScene == ptrScene) {
			ptr_delete(*iScene);
			return;
		}
	}
}

//*******************************************************************
//Scene
//*******************************************************************
Scene::Scene(SceneManager* manager) {
	manager_ = manager;
	type_ = Type::Unknown;
	bEnableRender_ = true;
	bEnableUpdate_ = true;
}
Scene::~Scene() {
}
void Scene::Render() {
	if (!bEnableRender_) return;
	for (auto itr = listTask_.begin(); itr != listTask_.end(); ++itr) {
		shared_ptr<TaskBase> task = *itr;
		if (task && !task->IsFinished())
			task->Render();
	}
}
void Scene::Update() {
	if (!bEnableUpdate_) return;
	for (auto itr = listTask_.begin(); itr != listTask_.end();) {
		shared_ptr<TaskBase> task = *itr;
		if (task && !task->IsFinished()) {
			task->Update();
			if (task->GetFrame() >= task->GetFrameEnd())
				task->bFinish_ = true;
			++itr;
		}
		else itr = listTask_.erase(itr);
	}
}
std::list<shared_ptr<TaskBase>>::iterator Scene::AddTask(shared_ptr<TaskBase> task) {
	listTask_.push_back(task);
	return listTask_.rbegin().base();
}
std::list<shared_ptr<TaskBase>>::iterator Scene::AddTask(std::list<shared_ptr<TaskBase>>::iterator itr, shared_ptr<TaskBase> task) {
	auto itrRes = listTask_.insert(itr, task);
	return itrRes;
}

//*******************************************************************
//TaskBase
//*******************************************************************
TaskBase::TaskBase(Scene* parent) {
	parent_ = parent;
	frame_ = 0;
	frameEnd_ = UINT_MAX;
	bFinish_ = false;
}
TaskBase::~TaskBase() {
}
