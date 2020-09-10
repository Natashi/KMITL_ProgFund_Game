#pragma once
#include "../../pch.h"

class Scene;
class TaskBase {
	friend class Scene;
public:
	TaskBase(Scene* parent);
	virtual ~TaskBase();

	virtual void Render() = 0;
	virtual void Update() = 0;

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
	enum class Type : uint8_t {
		Menu,
		StageUI,
		Stage,
		Background,
		Pause,
		Unknown,
	};
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

	void SetType(Type t) { type_ = t; }
	Type GetType() { return type_; }

	size_t GetTaskCount() { return listTask_.size(); }
private:
	SceneManager* manager_;
	Type type_;
	bool bEnableRender_;
	bool bEnableUpdate_;
	std::list<TaskBase*> listTask_;
};

class SceneManager {
	static SceneManager* base_;
public:
	enum {
		MAX_SCENE_SPACE = 8,
	};
public:
	SceneManager();
	~SceneManager();

	static SceneManager* const GetBase() { return base_; }

	void Initialize();

	void Render();
	void Update();

	void AddScene(Scene* ptrScene, size_t indexScene);
	void RemoveScene(size_t indexScene);
	void RemoveScene(Scene* ptrScene);
private:
	std::vector<Scene*> listScene_;
};