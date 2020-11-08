#pragma once
#include "../../pch.h"

#include "GConstant.hpp"

#include "ObjMove.hpp"

#define DEF_SHOTCONST_SET(s, i) \
	Red##s = i + 0, Purple##s = i + 1, Blue##s = i + 2, Cyan##s = i + 3, \
	Green##s = i + 4, Yellow##s = i + 5, Orange##s = i + 6, White##s = i + 7,

enum ShotConst : int {
	DEF_SHOTCONST_SET(Delay, 1000)

	DEF_SHOTCONST_SET(BallS, 0)
	DEF_SHOTCONST_SET(DarkBallS, 10)

	DEF_SHOTCONST_SET(BallM, 20)
	DEF_SHOTCONST_SET(RingBallM, 30)
	DEF_SHOTCONST_SET(RiceS, 40)
	DEF_SHOTCONST_SET(DarkRiceS, 50)
	DEF_SHOTCONST_SET(Scale, 60)
	DEF_SHOTCONST_SET(Kunai, 70)
	DEF_SHOTCONST_SET(GemS, 80)
	DEF_SHOTCONST_SET(Bullet, 90)
	DEF_SHOTCONST_SET(Amulet, 100)
	DEF_SHOTCONST_SET(Drop, 110)
	DEF_SHOTCONST_SET(StarS, 120)
	DEF_SHOTCONST_SET(Coin, 130)

	DEF_SHOTCONST_SET(BallL, 140)
	DEF_SHOTCONST_SET(RiceL, 150)
	DEF_SHOTCONST_SET(Butterfly, 160)
	DEF_SHOTCONST_SET(Knife, 170)
	DEF_SHOTCONST_SET(Heart, 180)
	DEF_SHOTCONST_SET(Arrow, 190)
	DEF_SHOTCONST_SET(StarL, 200)

	DEF_SHOTCONST_SET(Bubble, 210)
	DEF_SHOTCONST_SET(MikoOrb, 220)

	DEF_SHOTCONST_SET(Fire, 230)
	DEF_SHOTCONST_SET(Note, 240)

	DEF_SHOTCONST_SET(StLaser, 250)
	DEF_SHOTCONST_SET(CrLaser, 260)
};

enum ShotPlayerConst : int {
	Main,
	_Main,
	Homing,
	_Homing,
	Needle,
	NeedleB,
};

enum class ShotPolarity : byte {
	None,
	White,
	Black,
};
enum class ShotOwnerType : byte {
	None,
	Player,
	Enemy,
};

class Stage_ShotAnimation;
class Stage_ObjShot;

class Stage_ShotRenderer;

class Stage_MainScene;
class Stage_ShotManager : public TaskBase {
public:
	enum {
		SHOT_MAX = 8192,
	};
	typedef std::unordered_map<BlendMode, Stage_ShotRenderer*> ListRenderer;
private:
	std::list<shared_ptr<Stage_ObjShot>> listShotEnemy_;
	std::list<shared_ptr<Stage_ObjShot>> listShotPlayer_;

	std::map<int, Stage_ShotAnimation*> mapShotDataEnemy_;
	std::map<int, Stage_ShotAnimation*> mapShotDataPlayer_;

	std::list<std::pair<shared_ptr<TextureResource>, ListRenderer*>> listShotRendererSet_;
	shared_ptr<ShaderResource> shaderLayer_;

	DxRect<int> rcClip_;
public:
	Stage_ShotManager(Scene* parent);
	~Stage_ShotManager();

	virtual void Render();
	virtual void Update();

	void LoadEnemyShotData();
	void LoadPlayerShotData();
	Stage_ShotAnimation* GetEnemyShotData(int id);
	Stage_ShotAnimation* GetPlayerShotData(int id);

	void AddEnemyShot(shared_ptr<Stage_ObjShot> obj, ShotPolarity polarity);
	void AddPlayerShot(shared_ptr<Stage_ObjShot> obj, ShotPolarity polarity);
	shared_ptr<Stage_ObjShot> CreateShotA1(CD3DXVECTOR2 pos, double speed, double angle, int graphic, size_t delay);

	void DeleteInCircle(ShotOwnerType typeOwner, int cx, int cy, int radius);

	size_t GetEnemyShotCount() { return listShotEnemy_.size(); }
	size_t GetPlayerShotCount() { return listShotPlayer_.size(); }

	void SetClip(const DxRect<int>& clip) { rcClip_ = clip; }
	const DxRect<int>* GetClip() { return &rcClip_; }
};

class Stage_ShotRenderer : public RenderObject {
	friend class Stage_ShotManager;
protected:
	size_t countMaxVertex_;
	size_t countRenderVertex_;
	size_t countMaxIndex_;
	size_t countRenderIndex_;
protected:
	bool _TryExpandVertex(size_t chk);
	bool _TryExpandIndex(size_t chk);
public:
	Stage_ShotRenderer();

	virtual HRESULT Render() { return DSERR_UNSUPPORTED; }
	virtual HRESULT Render(Stage_ShotManager* manager);
	virtual void Update() {}

	void AddSquareVertex(VertexTLX* listVertex);

	size_t GetVertexCount() {
		return std::min(countRenderVertex_, vertex_.size());
	}
	void SetVertexCount(size_t count) {
		vertex_.resize(count);
	}
};

class Stage_ShotAnimation {
	friend class Stage_ShotManager;
public:
	struct Frame {
		DxRect<float> rcSrc;
		DxRect<float> rcDst;
		size_t frame;
	};
private:
	Stage_ShotAnimation* pDelayData_;
	Stage_ShotManager::ListRenderer* pAttachedRenderer_;

	shared_ptr<TextureResource> texture_;
	LONG width_;
	LONG height_;

	std::vector<Frame> listFrameData_;
	size_t maxFrame_;
public:
	double spin_;
	bool bFixedAngle_;
public:
	Stage_ShotAnimation(shared_ptr<TextureResource> texture, double spin, bool fixedAngle);
	Stage_ShotAnimation(shared_ptr<TextureResource> texture) : Stage_ShotAnimation(texture, 0, false) {};

	void AddFrame(const Frame& nFrame) {
		listFrameData_.push_back(nFrame);
		maxFrame_ += nFrame.frame;
	}

	Frame* GetFrame(size_t frame);
	size_t GetFrameCount() { return listFrameData_.size(); }

	LONG GetWidth() { return width_; }
	LONG GetHeight() { return height_; }

	Stage_ShotAnimation* GetDelayData() { return pDelayData_; }
	Stage_ShotManager::ListRenderer* GetAttachedRenderer() { return pAttachedRenderer_; }
	shared_ptr<TextureResource> GetTexture() { return texture_; }
};

class Stage_ObjShot : public RenderObject, public Stage_ObjMove {
	friend class Stage_ShotManager;
protected:
	Stage_ShotManager* shotManager_;
public:
	ShotPolarity polarity_;
	ShotOwnerType typeOwner_;

	bool bDelete_;

	size_t frame_;
	size_t frameDelay_;
	size_t frameDelayMax_;
	size_t frameClipImmune_;	//The shot will not be auto-deleted when going off the screen if this is > 0
	size_t frameFadeDelete_;	//The shot will be deleted once this reaches 0

	Stage_ShotAnimation* pShotData_;
	double shotAngleZ_;

	double life_;
	double damage_;
protected:
	virtual void _DeleteInLife();
	virtual void _DeleteInAutoClip();
	virtual void _DeleteInFade();
	virtual void _CommonUpdate();
public:
	Stage_ObjShot(Stage_ShotManager* manager);
	virtual ~Stage_ObjShot();

	virtual HRESULT Render();
	virtual void Update();

	size_t GetFrame() { return frame_; }

	void SetShotData(int id);
	void SetShotData(Stage_ShotAnimation* id);

	virtual void SetTexture(shared_ptr<TextureResource> texture) { texture_ = texture; }
	virtual void SetShader(shared_ptr<ShaderResource> shader) { shader_ = shader; }

	void SetSourceRectNormalized(VertexTLX* vert, DxRect<float>* rc);
	void SetDestRect(VertexTLX* vert, DxRect<float>* rc);
	void SetColor(VertexTLX* vert, D3DCOLOR color);

	void SetAngleZOff(double z) { shotAngleZ_ = z; }

	//bool CanIntersect() { return bIntersectionEnable_; }
	//virtual void Intersect(shared_ptr<Stage_ObjIntersect> ownRef, shared_ptr<Stage_ObjIntersect> otherRef);
};