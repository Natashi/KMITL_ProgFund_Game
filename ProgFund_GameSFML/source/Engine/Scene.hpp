#pragma once
#include "../../pch.h"

class Scene;
class TaskBase {
	friend class Scene;
public:
	TaskBase(Scene* parent);
	virtual ~TaskBase();

	virtual void Render() {};
	virtual void Update() {};

	Scene* GetParent() { return parent_; }

	void SetEndFrame(size_t frame) { frameEnd_ = frame; }

	size_t GetFrame() { return frame_; }
	size_t GetFrameEnd() { return frameEnd_; }
	bool IsFinished() { return bFinish_; }
protected:
	Scene* parent_;
	size_t frame_;
	size_t frameEnd_;
	bool bFinish_;
};

class SceneManager;
class Scene {
	friend class SceneManager;
public:
	typedef enum : uint8_t {
		Menu,
		Background,
		Stage,
		StageUI,
		Pause,
		Unknown,
	} Type;
public:
	Scene(SceneManager* manager);
	virtual ~Scene();

	virtual void Render();
	virtual void Update();

	SceneManager* GetParentManager() { return manager_; }

	void SetEnableRender(bool b) { bEnableRender_ = b; }
	bool IsEnableRender() { return bEnableRender_; }
	void SetEnableUpdate(bool b) { bEnableUpdate_ = b; }
	bool IsEnableUpdate() { return bEnableUpdate_; }

	bool IsDelete() { return bDelete_; }
	bool IsAutoDelete() { return bAutoDelete_; }

	void SetType(Type t) { type_ = t; }
	Type GetType() { return type_; }

	size_t GetFrame() { return frame_; }

	size_t GetTaskCount() { return listTask_.size(); }

	std::list<shared_ptr<TaskBase>>::iterator AddTask(shared_ptr<TaskBase> task);
	std::list<shared_ptr<TaskBase>>::iterator AddTask(std::list<shared_ptr<TaskBase>>::iterator itr, shared_ptr<TaskBase> task);
protected:
	SceneManager* manager_;
	size_t frame_;
	Type type_;
	bool bEnableRender_;
	bool bEnableUpdate_;
	bool bDelete_;
	bool bAutoDelete_;
	std::list<shared_ptr<TaskBase>> listTask_;
};

class SceneManager {
	static SceneManager* base_;
public:
	enum {
		MAX_SCENE_SPACE = 12,
	};
public:
	SceneManager();
	~SceneManager();

	static SceneManager* const GetBase() { return base_; }

	void Initialize();
	void Release();

	void Render();
	void Update();

	shared_ptr<Scene> GetPrimaryScene() { return primaryScene_; }
	shared_ptr<Scene> GetRearScene() { return rearScene_; }

	void AddScene(shared_ptr<Scene> ptrScene, size_t indexScene, bool bReplace = false);
	void RemoveScene(size_t indexScene);
	void RemoveScene(shared_ptr<Scene> ptrScene) { RemoveScene(ptrScene.get()); }
	void RemoveScene(Scene* ptrScene);

	bool IsAnyActive();
private:
	shared_ptr<Scene> primaryScene_;
	shared_ptr<Scene> rearScene_;
	std::vector<shared_ptr<Scene>> listScene_;
};