#pragma once
#include "../../pch.h"

#include "GConstant.hpp"

#include "Score.hpp"
#include "System.hpp"

class Pause_MainScene;

struct Pause_CallParams {
	std::string head;
	D3DCOLOR headColorTop;
	D3DCOLOR headColorBot;
	std::vector<std::string> desc;
	int type;
};
class Pause_PauseTask : public TaskBase {
	friend class Pause_MainScene;
private:
	Pause_CallParams callParams_;

	StaticRenderObject2D objHeader_;
	std::vector<shared_ptr<StaticRenderObject2D>> objDescription_;

	bool bEnd_;
	int typePause_;
	bool flgGetInput_;
public:
	Pause_PauseTask(Scene* parent, const Pause_CallParams& params);

	virtual void Render(byte layer);
	virtual void Update();
};
class Pause_NameRegistTask : public TaskBase {
	friend class Pause_MainScene;
private:
	static const size_t MAX_NAME_SIZE = 8U;
private:
	Pause_CallParams prevMenuData_;

	size_t frameHoldDelay_;
	bool bEnd_;
	int selectIndex_;

	std::vector<Sprite2D> objNameDisplay_;
	std::string name_;
	size_t nameLength_;

	PlayerStats statPlayer_;
	std::vector<StaticRenderObject2D> objPlayerStats_;

	std::vector<std::pair<Sprite2D, char>> listAllChars_;
public:
	Pause_NameRegistTask(Scene* parent, const Pause_CallParams& prevMenuParams);

	virtual void Render(byte layer);
	virtual void Update();
};

class Pause_MainScene : public Scene {
private:
	bool bPause_;

	shared_ptr<TextureResource> pRenderTarget_;
	Sprite2D objBack_;
	Sprite2D objBlack_;
	double tAlpha_;
public:
	Pause_MainScene(SceneManager* manager);
	~Pause_MainScene();

	virtual void Render();
	virtual void Update();

	void EnterPause(int type);
	void LeavePause();
};