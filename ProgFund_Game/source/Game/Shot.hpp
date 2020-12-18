#pragma once
#include "../../pch.h"

#include "GConstant.hpp"

#include "ObjMove.hpp"
#include "Intersection.hpp"

#define DEF_SHOTCONST_SET(s, i) \
	Red##s = (i) + 0, Purple##s = (i) + 1, Blue##s = (i) + 2, Cyan##s = (i) + 3, \
	Green##s = (i) + 4, Yellow##s = (i) + 5, Orange##s = (i) + 6, White##s = (i) + 7,
#define DEF_SHOTCONST_SET_I8(s, i) DEF_SHOTCONST_SET(s, i * 8)

enum ShotConst : int {
	DEF_SHOTCONST_SET_I8(, 0)

	DEF_SHOTCONST_SET(Delay, 1000)

	DEF_SHOTCONST_SET_I8(BallS, 0)
	DEF_SHOTCONST_SET_I8(DarkBallS, 1)

	DEF_SHOTCONST_SET_I8(BallM, 2)
	DEF_SHOTCONST_SET_I8(RingBallM, 3)
	DEF_SHOTCONST_SET_I8(RiceS, 4)
	DEF_SHOTCONST_SET_I8(DarkRiceS, 5)
	DEF_SHOTCONST_SET_I8(Scale, 6)
	DEF_SHOTCONST_SET_I8(Kunai, 7)
	DEF_SHOTCONST_SET_I8(GemS, 8)
	DEF_SHOTCONST_SET_I8(Bullet, 9)
	DEF_SHOTCONST_SET_I8(Amulet, 10)
	DEF_SHOTCONST_SET_I8(Drop, 11)
	DEF_SHOTCONST_SET_I8(StarS, 12)
	DEF_SHOTCONST_SET_I8(Coin, 13)

	DEF_SHOTCONST_SET_I8(BallL, 14)
	DEF_SHOTCONST_SET_I8(RingBallL, 15)
	DEF_SHOTCONST_SET_I8(RiceL, 16)
	DEF_SHOTCONST_SET_I8(Butterfly, 17)
	DEF_SHOTCONST_SET_I8(Knife, 18)
	DEF_SHOTCONST_SET_I8(Heart, 19)
	DEF_SHOTCONST_SET_I8(Arrow, 20)
	DEF_SHOTCONST_SET_I8(StarL, 21)

	DEF_SHOTCONST_SET_I8(Bubble, 22)
	DEF_SHOTCONST_SET_I8(MikoOrb, 23)

	DEF_SHOTCONST_SET_I8(Fire, 24)
	DEF_SHOTCONST_SET_I8(Note, 25)

	DEF_SHOTCONST_SET_I8(StLaser, 26)
	DEF_SHOTCONST_SET_I8(CrLaser, 27)
};

enum ShotPlayerConst : int {
	Main,
	_Main,
	Homing,
	_Homing,
	Needle,
	NeedleB,
};

enum class ShotOwnerType : byte {
	None,
	Player,
	Enemy,
};

class Stage_ShotAnimation;
class Stage_ObjShot;

class Stage_ShotRenderer;
class Stage_ShotDeleteEffectRendererTask;

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

	DxRectangle<int> rcClip_;

	shared_ptr<Stage_ShotDeleteEffectRendererTask> pTaskDeleteEffect_;
public:
	Stage_ShotManager(Scene* parent);
	~Stage_ShotManager();

	virtual void Render(byte layer);
	virtual void Update();

	void LoadEnemyShotData();
	void LoadPlayerShotData();
	Stage_ShotAnimation* GetEnemyShotData(int id);
	Stage_ShotAnimation* GetPlayerShotData(int id);

	shared_ptr<Stage_ObjShot> CreateShotA1(ShotOwnerType typeOwner, CD3DXVECTOR2 pos,
		double speed, double angle, int graphic, size_t delay);
	void AddEnemyShot(shared_ptr<Stage_ObjShot> obj, IntersectPolarity polarity);
	shared_ptr<Stage_ObjShot> AddEnemyShot(CD3DXVECTOR2 pos, double speed, double angle,
		int graphic, size_t delay, IntersectPolarity polarity);
	void AddPlayerShot(shared_ptr<Stage_ObjShot> obj, IntersectPolarity polarity);
	shared_ptr<Stage_ObjShot> AddPlayerShot(CD3DXVECTOR2 pos, double speed, double angle,
		int graphic, size_t delay, IntersectPolarity polarity);

	void DeleteInCircle(ShotOwnerType typeOwner, int64_t cx, int64_t cy, int64_t radius, bool bForce = false);

	size_t GetEnemyShotCount() { return listShotEnemy_.size(); }
	size_t GetPlayerShotCount() { return listShotPlayer_.size(); }

	void SetClip(const DxRectangle<int>& clip) { rcClip_ = clip; }
	const DxRectangle<int>* GetClip() { return &rcClip_; }
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

	virtual HRESULT Render();
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
		DxRectangle<float> rcSrc;
		DxRectangle<float> rcDst;
		size_t frame;
	};
private:
	Stage_ShotAnimation* pDelayData_;
	Stage_ShotManager::ListRenderer* pAttachedRenderer_;

	shared_ptr<TextureResource> texture_;
	LONG width_;
	LONG height_;

	D3DCOLOR color_;

	std::vector<Frame> listFrameData_;
	size_t maxFrame_;

	DxCircle<float> hitCircle_;
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

	const Frame* GetFrame(size_t frame) const;
	size_t GetFrameCount() const { return listFrameData_.size(); }

	LONG GetWidth() const { return width_; }
	LONG GetHeight() const { return height_; }

	D3DCOLOR GetColor() const { return color_; }

	Stage_ShotAnimation* GetDelayData() const { return pDelayData_; }
	Stage_ShotManager::ListRenderer* GetAttachedRenderer() const { return pAttachedRenderer_; }
	shared_ptr<TextureResource> GetTexture() const { return texture_; }

	Stage_ShotRenderer* GetRendererFromBlendType(BlendMode blend);

	const DxCircle<float>* GetHitboxCircle() const { return &hitCircle_; }
};

class Stage_ObjShot : public RenderObject, public Stage_ObjMove, public Stage_ObjCollision {
	friend class Stage_ShotManager;
public:
	enum {
		FADE_MAX = 12,
	};
protected:
	Stage_ShotManager* shotManager_;
public:
	weak_ptr<Stage_ObjShot> pOwnRefWeak_;

	IntersectPolarity polarity_;
	ShotOwnerType typeOwner_;

	size_t frame_;
	size_t frameDelay_;
	size_t frameDelayMax_;
	size_t frameClipImmune_;	//The shot will not be auto-deleted when going off the screen if this is > 0
	int frameFadeDelete_;		//The shot will be deleted once this reaches 0

	Stage_ShotAnimation* pShotData_;
	double shotAngleZ_;

	double life_;
	double damage_;

	shared_ptr<Stage_IntersectionTarget> pIntersectionTarget_;
protected:
	virtual void _DeleteInLife();
	virtual void _DeleteInAutoClip();
	virtual void _DeleteInFade();
	virtual void _CommonUpdate();

	virtual void _RegistIntersection();

	virtual void _LoadVertices(const DxRectangle<float>* rcSrc, const DxRectangle<float>* rcDst, 
		D3DCOLOR color, float scale, CD3DXVECTOR2 pos);
public:
	Stage_ObjShot(Stage_ShotManager* manager);
	virtual ~Stage_ObjShot();

	virtual HRESULT Render();
	virtual void Update();

	size_t GetFrame() { return frame_; }

	IntersectPolarity GetPolarity() { return polarity_; }
	ShotOwnerType GetOwnerType() { return typeOwner_; }

	void SetShotData(int id);
	void SetShotData(Stage_ShotAnimation* id);
	Stage_ShotAnimation* GetShotData() { return pShotData_; }

	virtual void SetTexture(shared_ptr<TextureResource> texture) { texture_ = texture; }
	virtual void SetShader(shared_ptr<ShaderResource> shader) { shader_ = shader; }

	void SetSourceRectNormalized(VertexTLX* vert, const DxRectangle<float>* rc);
	void SetDestRect(VertexTLX* vert, const DxRectangle<float>* rc);
	void SetColor(VertexTLX* vert, D3DCOLOR color);

	void SetAngleZOff(double z) { shotAngleZ_ = z; }

	//bool CanIntersect() { return bIntersectionEnable_; }
	virtual void Intersect(Stage_IntersectionTarget* ownTarget, Stage_IntersectionTarget* otherTarget);
};

//--------------------------------------------------------------------------

class Stage_ShotDeleteEffectRendererTask : public TaskBase {
	friend class Stage_ShotManager;
public:
	struct ParticleData {
		D3DXVECTOR2 pos;
		D3DXVECTOR2 move;
		D3DXVECTOR2 angle;
		D3DCOLOR color;
		float scale;
		size_t life;
	};
	static constexpr size_t FRAME_PER_STEP = 3;
protected:
	shared_ptr<TextureResource> texture_;

	SpriteInstanced2D objEffects_;
	std::list<ParticleData*> listParticle_;
public:
	Stage_ShotDeleteEffectRendererTask(Scene* parent);

	virtual void Render(byte layer);
	virtual void Update();

	void AddInstance(Stage_ObjShot* shot);
};