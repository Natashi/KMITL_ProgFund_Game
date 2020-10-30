#pragma once
#include "../../pch.h"

#include "GConstant.hpp"

#include "System.hpp"

//Splash task
class Menu_SplashScene : public Scene {
public:
	Menu_SplashScene(SceneManager* manager);
	~Menu_SplashScene();
};
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
class Menu_MainScene : public Scene {
public:
	Menu_MainScene(SceneManager* manager);
	~Menu_MainScene();

	virtual void Render();
	virtual void Update();

	void SaveConfig();
	void LoadConfig();
private:
	shared_ptr<SoundResource> musicBackground_;
	Sprite2D objBackground_;
};

class Menu_Child_RankMenu;

class Menu_Child_ParentMenu_Item;
class Menu_Child_ParentMenu : public TaskBase {
public:
	Menu_Child_ParentMenu(Menu_MainScene* parent);
	~Menu_Child_ParentMenu();

	virtual void Render();
	virtual void Update();
protected:
	std::vector<Menu_Child_ParentMenu_Item*> listMenuObj_;
	bool flgGetInput_ = false;
	int selectIndex_;
};

class Menu_Child_RankMenu_Item;
class Menu_Child_RankMenu : public TaskBase {
public:
	Menu_Child_RankMenu(Menu_MainScene* parent);
	~Menu_Child_RankMenu();

	virtual void Render();
	virtual void Update();
protected:
	std::vector<Menu_Child_RankMenu_Item*> listRankObj_;
	bool flgGetInput_ = false;
	int selectIndex_;
};

class Menu_Child_OptionMenu_Item_Sound;
class Menu_Child_OptionMenu_Item_Stat;
class Menu_Child_OptionMenu : public TaskBase {
public:
	Menu_Child_OptionMenu(Menu_MainScene* parent);
	~Menu_Child_OptionMenu();

	virtual void Render();
	virtual void Update();
protected:
	std::vector<Menu_Child_OptionMenu_Item_Sound*> listOptionObjSound_;
	std::vector<Menu_Child_OptionMenu_Item_Stat*> listOptionObjStat_;
	bool flgGetInput_ = false;
	int selectIndex_;
};