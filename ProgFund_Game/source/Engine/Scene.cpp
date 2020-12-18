#include "pch.h"
#include "Utility.hpp"
#include "Scene.hpp"

//*******************************************************************
//SceneManager
//*******************************************************************
SceneManager* SceneManager::base_ = nullptr;
SceneManager::SceneManager() {
	primaryScene_ = nullptr;
	rearScene_ = nullptr;
}
SceneManager::~SceneManager() {
}
void SceneManager::Initialize() {
	if (base_) throw EngineError("SceneManager already initialized.");
	base_ = this;

	primaryScene_ = std::make_shared<Scene>(this);
	rearScene_ = std::make_shared<Scene>(this);
	listScene_.resize(MAX_SCENE_SPACE);
}
void SceneManager::Release() {
	listScene_.clear();
}
void SceneManager::Render() {
	primaryScene_->Render();

	for (shared_ptr<Scene>& iScene : listScene_) {
		if (iScene && !iScene->IsToBeDeleted()) {
			if (iScene->bEnableRender_) iScene->Render();
		}
	}

	rearScene_->Render();
}
void SceneManager::Update() {
	primaryScene_->Update();

	for (shared_ptr<Scene>& iScene : listScene_) {
		if (iScene) {
			if (iScene->bEnableUpdate_) iScene->Update();
			if (iScene->IsToBeDeleted())
				iScene = nullptr;
		}
	}

	rearScene_->Update();
}
void SceneManager::AddScene(shared_ptr<Scene> ptrScene, size_t indexScene, bool bReplace) {
	if (indexScene >= MAX_SCENE_SPACE)
		throw EngineError(StringUtility::Format("Scene index out of bounds: %u", indexScene));
	else if (!bReplace && listScene_[indexScene])
		throw EngineError(StringUtility::Format("Scene already exists: %u", indexScene));
	listScene_[indexScene] = ptrScene;
}
void SceneManager::RemoveScene(size_t indexScene) {
	shared_ptr<Scene>& pScene = listScene_[indexScene];
	if (pScene == nullptr) return;
	pScene->bDelete_ = true;
	pScene->bEnableRender_ = false;
	pScene->bEnableUpdate_ = false;
	//pScene = nullptr;
}
void SceneManager::RemoveScene(Scene* ptrScene) {
	for (shared_ptr<Scene>& iScene : listScene_) {
		if (iScene.get() == ptrScene) {
			iScene->bDelete_ = true;
			iScene->bEnableRender_ = false;
			iScene->bEnableUpdate_ = false;
			//iScene = nullptr;
			return;
		}
	}
}
bool SceneManager::IsAnyActive() {
	for (shared_ptr<Scene>& iScene : listScene_) {
		if (iScene)
			return true;
	}
	return false;
}

//*******************************************************************
//Scene
//*******************************************************************
Scene::Scene(SceneManager* manager) {
	manager_ = manager;
	frame_ = 0U;
	type_ = Type::Unknown;
	bEnableRender_ = true;
	bEnableUpdate_ = true;
	bDelete_ = false;
	bAutoDelete_ = true;
}
Scene::~Scene() {
}
void Scene::Render() {
	for (byte iLayer = 0; iLayer < MAX_RENDER_LAYER; ++iLayer) {
		for (shared_ptr<TaskBase>& iTask : listTask_) {
			if (iTask && !iTask->IsFinished())
				iTask->Render(iLayer);
		}
	}
}
void Scene::Update() {
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
	++frame_;
}
std::list<shared_ptr<TaskBase>>::iterator Scene::AddTask(shared_ptr<TaskBase> task) {
	return listTask_.insert(listTask_.end(), task);
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
