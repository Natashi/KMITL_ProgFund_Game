#include "pch.h"

#include "Shot.hpp"
#include "StageMain.hpp"

//*******************************************************************
//Stage_ShotManager
//*******************************************************************
Stage_ShotManager::Stage_ShotManager(Scene* parent) : TaskBase(parent) {
	GET_INSTANCE(ResourceManager, resourceManager);

	resourceManager->LoadResource<TextureResource>("resource/img/stage/eff_shotbreak.png", "img/stage/eff_shotbreak.png");

	//---------------------------------------------------------------------

	shaderLayer_ = resourceManager->LoadResource<ShaderResource>("resource/shader/layer_shot.fx", "shader/layer_shot.fx");

	resourceManager->LoadResource<ShaderResource>("resource/shader/instanced_shotbreak.fx", "shader/instanced_shotbreak.fx");

	{
		pTaskDeleteEffect_ = shared_ptr<Stage_ShotDeleteEffectRendererTask>(new Stage_ShotDeleteEffectRendererTask(parent));
		parent->AddTask(pTaskDeleteEffect_);
	}
}
Stage_ShotManager::~Stage_ShotManager() {
	for (auto itr = mapShotDataEnemy_.begin(); itr != mapShotDataEnemy_.end(); ++itr)
		ptr_delete(itr->second);
	for (auto itr = mapShotDataPlayer_.begin(); itr != mapShotDataPlayer_.end(); ++itr)
		ptr_delete(itr->second);

	for (auto itrSet = listShotRendererSet_.begin(); itrSet != listShotRendererSet_.end(); ++itrSet) {
		auto& pList = itrSet->second;
		for (auto itrList = pList->begin(); itrList != pList->end(); ++itrList) {
			ptr_delete(itrList->second);
		}
		ptr_delete(pList);
	}
}

#define LOAD_RENDERER_LIST(ls) \
	ls->insert(std::make_pair(BlendMode::Add, new Stage_ShotRenderer())); \
	ls->insert(std::make_pair(BlendMode::Subtract, new Stage_ShotRenderer())); \
	ls->insert(std::make_pair(BlendMode::RevSubtract, new Stage_ShotRenderer())); \
	ls->insert(std::make_pair(BlendMode::Invert, new Stage_ShotRenderer())); \
	ls->insert(std::make_pair(BlendMode::Alpha, new Stage_ShotRenderer()));

void Stage_ShotManager::LoadEnemyShotData() {
	GET_INSTANCE(ResourceManager, resourceManager);

	auto textureDelay = resourceManager->LoadResource<TextureResource>(
		"resource/img/stage/stg_shots_delay.png", "img/stage/stg_shots_delay.png");
	auto textureEnemy = resourceManager->LoadResource<TextureResource>(
		"resource/img/stage/stg_shots.png", "img/stage/stg_shots.png");

	ListRenderer* rendererDelay = new ListRenderer();
	LOAD_RENDERER_LIST(rendererDelay);
	listShotRendererSet_.push_back(std::make_pair(textureDelay, rendererDelay));

	ListRenderer* rendererEnemy = new ListRenderer();
	LOAD_RENDERER_LIST(rendererEnemy);
	listShotRendererSet_.push_back(std::make_pair(textureEnemy, rendererEnemy));

	D3DCOLOR listShotColor[] = {
		D3DCOLOR_XRGB(255, 32, 32),		//Red
		D3DCOLOR_XRGB(255, 32, 255),	//Magenta
		D3DCOLOR_XRGB(32, 32, 255),		//Blue
		D3DCOLOR_XRGB(32, 255, 255),	//Cyan
		D3DCOLOR_XRGB(32, 255, 32),		//Green
		D3DCOLOR_XRGB(255, 255, 32),	//Yellow
		D3DCOLOR_XRGB(255, 128, 32),	//Orange
		D3DCOLOR_XRGB(192, 192, 192),	//Grey
	};

	Stage_ShotAnimation* listDelayData[8];

	//Delays
	{
		int j = 0;
		for (int i = ShotConst::RedDelay; i < ShotConst::WhiteDelay + 1; ++i, ++j) {
			Stage_ShotAnimation* anime = new Stage_ShotAnimation(textureDelay);
			anime->pAttachedRenderer_ = rendererDelay;
			anime->color_ = listShotColor[j];

			Stage_ShotAnimation::Frame frame = {
				DxRectangle<float>::SetFromIndex(64, 64, i - ShotConst::RedDelay, 4),
				DxRectangle<float>(-32, -32, 32, 32), 1
			};
			anime->AddFrame(frame);

			mapShotDataEnemy_[i] = anime;
			listDelayData[j] = anime;
		}
	}

	constexpr const int S_STEP = 8;

	//Small shots
	{
		int iShotIni = ShotConst::RedBallS;
		for (int i = 0; i < (ShotConst::RedDarkBallS - ShotConst::RedBallS) / S_STEP + 1; ++i) {
			int yOff = 0 + i * 8;
			for (int c = 0; c < 8; ++c) {
				Stage_ShotAnimation* anime = new Stage_ShotAnimation(textureEnemy, 0, true);
				anime->pDelayData_ = listDelayData[c];
				anime->pAttachedRenderer_ = rendererEnemy;
				anime->color_ = listShotColor[c];

				Stage_ShotAnimation::Frame frame = {
					DxRectangle<float>::SetFromIndex(8, 8, c, 8, 0, yOff),
					DxRectangle<float>(-4, -4, 4, 4), 1
				};
				anime->AddFrame(frame);

				mapShotDataEnemy_[iShotIni + i * S_STEP + c] = anime;
			}
		}
	}

	//Medium shots
	{
		double listSpin[] = {
			0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			6, 7,
		};
		bool listFixedAngle[] = {
			true, true,
			false, false, false, false,
			false, false, false, false,
			false, false,
		};

		int iShotIni = ShotConst::RedBallM;
		int iType = 0;
		for (int i = 0; i < (ShotConst::RedCoin - ShotConst::RedBallM) / S_STEP + 1; ++i, ++iType) {
			int yOff = 16 + i * 16;
			for (int c = 0; c < 8; ++c) {
				Stage_ShotAnimation* anime = new Stage_ShotAnimation(textureEnemy, 
					listSpin[iType], listFixedAngle[iType]);
				anime->pDelayData_ = listDelayData[c];
				anime->pAttachedRenderer_ = rendererEnemy;
				anime->color_ = listShotColor[c];

				Stage_ShotAnimation::Frame frame = {
					DxRectangle<float>::SetFromIndex(16, 16, c, 8, 0, yOff),
					DxRectangle<float>(-8, -8, 8, 8), 1
				};
				anime->AddFrame(frame);

				mapShotDataEnemy_[iShotIni + i * S_STEP + c] = anime;
			}
		}
	}

	//Large shots
	{
		double listSpin[] = {
			0, 0,
			0, 0, 0,
			0, 0, 6,
		};
		bool listFixedAngle[] = {
			true, true,
			false, false, false,
			false, false, false,
		};

		int iShotIni = ShotConst::RedBallL;
		int iType = 0;
		for (int i = 0; i < (ShotConst::RedStarL - ShotConst::RedBallL) / S_STEP + 1; ++i, ++iType) {
			int yOff = i * 32;
			for (int c = 0; c < 8; ++c) {
				Stage_ShotAnimation* anime = new Stage_ShotAnimation(textureEnemy, 
					listSpin[iType], listFixedAngle[iType]);
				anime->pDelayData_ = listDelayData[c];
				anime->pAttachedRenderer_ = rendererEnemy;
				anime->color_ = listShotColor[c];

				Stage_ShotAnimation::Frame frame = {
					DxRectangle<float>::SetFromIndex(32, 32, c, 8, 128, yOff),
					DxRectangle<float>(-16, -16, 16, 16), 1
				};
				anime->AddFrame(frame);

				mapShotDataEnemy_[iShotIni + i * S_STEP + c] = anime;
			}
		}
	}

	//Very large shots
	{
		int iShotIni = ShotConst::RedBubble;
		for (int i = 0; i < (ShotConst::RedMikoOrb - ShotConst::RedBubble) / S_STEP + 1; ++i) {
			int yOff = 256 + i * 64;
			for (int c = 0; c < 8; ++c) {
				Stage_ShotAnimation* anime = new Stage_ShotAnimation(textureEnemy);
				anime->pDelayData_ = listDelayData[c];
				anime->pAttachedRenderer_ = rendererEnemy;
				anime->color_ = listShotColor[c];

				Stage_ShotAnimation::Frame frame = {
					DxRectangle<float>::SetFromIndex(64, 64, c, 8, 0, yOff),
					DxRectangle<float>(-32, -32, 32, 32), 1
				};
				anime->AddFrame(frame);

				mapShotDataEnemy_[iShotIni + i * S_STEP + c] = anime;
			}
		}
	}

	//Animated shots
	{
		bool listFixedAngle[] = {
			false, true,
		};
		size_t listFrameDuration[] = {
			4, 9,
		};

		int iShotIni = ShotConst::RedFire;
		int iType = 0;
		for (int i = 0; i < (ShotConst::RedNote - ShotConst::RedFire) / S_STEP + 1; ++i, ++iType) {
			int xOff = 0 + i * (32 * 8);
			for (int c = 0; c < 8; ++c) {
				Stage_ShotAnimation* anime = new Stage_ShotAnimation(textureEnemy, 0, listFixedAngle[iType]);
				anime->pDelayData_ = listDelayData[c];
				anime->pAttachedRenderer_ = rendererEnemy;
				anime->color_ = listShotColor[c];

				Stage_ShotAnimation::Frame frame = {
					DxRectangle<float>::SetFromIndex(32, 32, c, 8, xOff, 384),
					DxRectangle<float>(-16, -16, 16, 16), listFrameDuration[iType]
				};
				anime->AddFrame(frame);
				frame.rcSrc += DxRectangle<float>(0, 32, 0, 32);
				anime->AddFrame(frame);
				frame.rcSrc += DxRectangle<float>(0, 32, 0, 32);
				anime->AddFrame(frame);
				frame.rcSrc -= DxRectangle<float>(0, 32, 0, 32);
				anime->AddFrame(frame);

				mapShotDataEnemy_[iShotIni + i * S_STEP + c] = anime;
			}
		}
	}

	//Lasers
	{
		int iShotIni = ShotConst::RedStLaser;
		for (int i = 0; i < (ShotConst::RedCrLaser - ShotConst::RedStLaser) / S_STEP + 1; ++i) {
			int yOff = 0 + i * 32;
			for (int c = 0; c < 8; ++c) {
				Stage_ShotAnimation* anime = new Stage_ShotAnimation(textureEnemy);
				anime->pDelayData_ = listDelayData[c];
				anime->pAttachedRenderer_ = rendererEnemy;
				anime->color_ = listShotColor[c];

				Stage_ShotAnimation::Frame frame = {
					DxRectangle<float>::SetFromIndex(16, 32, c, 8, 384, yOff),
					DxRectangle<float>(-8, -16, 8, 16), 1
				};
				anime->AddFrame(frame);

				mapShotDataEnemy_[iShotIni + i * S_STEP + c] = anime;
			}
		}
	}

	//Normalize rects and set intersection size
	for (auto itr = mapShotDataEnemy_.begin(); itr != mapShotDataEnemy_.end(); ++itr) {
		Stage_ShotAnimation* pShotData = itr->second;

		auto& texture = pShotData->texture_;
		auto& listFrame = pShotData->listFrameData_;
		for (size_t iFrame = 0; iFrame < listFrame.size(); ++iFrame) {
			listFrame[iFrame].rcSrc /= DxRectangle<float>::SetFromSize(texture->GetWidth(), texture->GetHeight());
			if (iFrame == 0) {
				pShotData->width_ = listFrame[iFrame].rcDst.GetWidth();
				pShotData->height_ = listFrame[iFrame].rcDst.GetHeight();
			}
			//iFrame.rcDst -= 0.5f;	//Bias
		}

		if (pShotData->hitCircle_.r < 0) {
			float rAvg = (pShotData->width_ + pShotData->height_) / 2.0f;
			if (rAvg > 0)
				pShotData->hitCircle_.r = std::max(rAvg / 3.0f, 2.0f) * 0.5f;
		}
	}
}
void Stage_ShotManager::LoadPlayerShotData() {
	GET_INSTANCE(ResourceManager, resourceManager);

	auto texturePlayer = resourceManager->LoadResource<TextureResource>(
		"resource/img/player/pl00.png", "img/player/pl00.png");

	ListRenderer* rendererPlayer = new ListRenderer();
	LOAD_RENDERER_LIST(rendererPlayer);
	listShotRendererSet_.push_front(std::make_pair(texturePlayer, rendererPlayer));

	//Main
	{
		Stage_ShotAnimation* anime = new Stage_ShotAnimation(texturePlayer);
		anime->pAttachedRenderer_ = rendererPlayer;

		Stage_ShotAnimation::Frame frame = {
			DxRectangle<float>::SetFromSize(192, 144, 64, 16),
			DxRectangle<float>(-32, -8, 32, 8), 1
		};
		anime->AddFrame(frame);

		mapShotDataPlayer_[ShotPlayerConst::Main] = anime;
	}
	{
		Stage_ShotAnimation* anime = new Stage_ShotAnimation(texturePlayer);
		anime->pAttachedRenderer_ = rendererPlayer;

		Stage_ShotAnimation::Frame frame = {
			DxRectangle<float>::SetFromSize(0, 144, 16, 16),
			DxRectangle<float>(-8, -8, 8, 8), 4
		};
		anime->AddFrame(frame);
		frame.rcSrc += DxRectangle<float>(16, 0, 16, 0);
		anime->AddFrame(frame);
		frame.rcSrc += DxRectangle<float>(16, 0, 16, 0);
		anime->AddFrame(frame);
		frame.rcSrc += DxRectangle<float>(16, 0, 16, 0);
		anime->AddFrame(frame);

		mapShotDataPlayer_[ShotPlayerConst::_Main] = anime;
	}

	//Homing
	{
		Stage_ShotAnimation* anime = new Stage_ShotAnimation(texturePlayer, 8, false);
		anime->pAttachedRenderer_ = rendererPlayer;

		Stage_ShotAnimation::Frame frame = {
			DxRectangle<float>::SetFromSize(0, 160, 16, 16),
			DxRectangle<float>(-8, -8, 8, 8), 1
		};
		anime->AddFrame(frame);

		mapShotDataPlayer_[ShotPlayerConst::Homing] = anime;
	}
	{
		Stage_ShotAnimation* anime = new Stage_ShotAnimation(texturePlayer, 12, false);
		anime->pAttachedRenderer_ = rendererPlayer;

		Stage_ShotAnimation::Frame frame = {
			DxRectangle<float>::SetFromSize(0, 160, 16, 16),
			DxRectangle<float>(-8, -8, 8, 8), 4
		};
		anime->AddFrame(frame);
		frame.rcSrc += DxRectangle<float>(16, 0, 16, 0);
		anime->AddFrame(frame);
		frame.rcSrc += DxRectangle<float>(16, 0, 16, 0);
		anime->AddFrame(frame);
		frame.rcSrc += DxRectangle<float>(16, 0, 16, 0);
		anime->AddFrame(frame);

		mapShotDataPlayer_[ShotPlayerConst::_Homing] = anime;
	}

	//Needle
	{
		Stage_ShotAnimation* anime = new Stage_ShotAnimation(texturePlayer);
		anime->pAttachedRenderer_ = rendererPlayer;

		Stage_ShotAnimation::Frame frame = {
			DxRectangle<float>::SetFromSize(0, 176, 64, 16),
			DxRectangle<float>(-32, -8, 32, 8), 5
		};
		anime->AddFrame(frame);
		frame.rcSrc += DxRectangle<float>(64, 0, 64, 0);
		anime->AddFrame(frame);

		mapShotDataPlayer_[ShotPlayerConst::Needle] = anime;
	}
	{
		Stage_ShotAnimation* anime = new Stage_ShotAnimation(texturePlayer);
		anime->pAttachedRenderer_ = rendererPlayer;

		Stage_ShotAnimation::Frame frame = {
			DxRectangle<float>::SetFromSize(0, 192, 64, 32),
			DxRectangle<float>(-32, -16, 32, 16), 5
		};
		anime->AddFrame(frame);
		frame.rcSrc += DxRectangle<float>(64, 0, 64, 0);
		anime->AddFrame(frame);

		mapShotDataPlayer_[ShotPlayerConst::NeedleB] = anime;
	}

	//Normalize rects and set intersection size
	for (auto itr = mapShotDataPlayer_.begin(); itr != mapShotDataPlayer_.end(); ++itr) {
		Stage_ShotAnimation* pShotData = itr->second;

		auto& texture = pShotData->texture_;
		auto& listFrame = pShotData->listFrameData_;
		for (size_t iFrame = 0; iFrame < listFrame.size(); ++iFrame) {
			D3DXIMAGE_INFO* imgInfo = texture->GetImageInfo();
			listFrame[iFrame].rcSrc /= DxRectangle<float>::SetFromSize(texture->GetWidth(), texture->GetHeight());
			if (iFrame == 0) {
				pShotData->width_ = listFrame[iFrame].rcDst.GetWidth();
				pShotData->height_ = listFrame[iFrame].rcDst.GetHeight();
			}
			//iFrame.rcDst -= 0.5f;	//Bias
		}

		if (pShotData->hitCircle_.r < 0) {
			float rAvg = (pShotData->width_ + pShotData->height_) / 2.0f;
			if (rAvg > 0)
				pShotData->hitCircle_.r = std::max(rAvg / 3.0f, 2.0f);
		}
	}
}

#undef LOAD_RENDERER_LIST

Stage_ShotAnimation* Stage_ShotManager::GetEnemyShotData(int id) {
	auto itr = mapShotDataEnemy_.find(id);
	if (itr == mapShotDataEnemy_.end()) return nullptr;
	return itr->second;
}
Stage_ShotAnimation* Stage_ShotManager::GetPlayerShotData(int id) {
	auto itr = mapShotDataPlayer_.find(id);
	if (itr == mapShotDataPlayer_.end()) return nullptr;
	return itr->second;
}

shared_ptr<Stage_ObjShot> Stage_ShotManager::CreateShotA1(ShotOwnerType typeOwner, CD3DXVECTOR2 pos, 
	double speed, double angle, int graphic, size_t delay) 
{
	shared_ptr<Stage_ObjShot> nShot = shared_ptr<Stage_ObjShot>(new Stage_ObjShot(this));
	nShot->pOwnRefWeak_ = weak_ptr(nShot);
	nShot->typeOwner_ = typeOwner;
	nShot->Stage_ObjMove::SetX(pos.x);
	nShot->Stage_ObjMove::SetY(pos.y);
	nShot->SetSpeed(speed);
	nShot->SetDirectionAngle(angle);
	nShot->SetShotData(graphic);
	nShot->frameDelay_ = delay;
	nShot->frameDelayMax_ = delay;
	return nShot;
}
void Stage_ShotManager::AddEnemyShot(shared_ptr<Stage_ObjShot> obj, IntersectPolarity polarity) {
	if (listShotEnemy_.size() >= 8192) return;
	obj->typeOwner_ = ShotOwnerType::Enemy;
	obj->polarity_ = polarity;
	listShotEnemy_.push_back(obj);
}
shared_ptr<Stage_ObjShot> Stage_ShotManager::AddEnemyShot(CD3DXVECTOR2 pos, double speed, double angle,
	int graphic, size_t delay, IntersectPolarity polarity)
{
	shared_ptr<Stage_ObjShot> shot = this->CreateShotA1(ShotOwnerType::Enemy, pos, speed, angle, graphic, delay);
	shot->polarity_ = polarity;
	if (listShotEnemy_.size() < 8192)
		listShotEnemy_.push_back(shot);
	return shot;
}
void Stage_ShotManager::AddPlayerShot(shared_ptr<Stage_ObjShot> obj, IntersectPolarity polarity) {
	if (listShotPlayer_.size() >= 1024) return;
	obj->typeOwner_ = ShotOwnerType::Player;
	obj->polarity_ = polarity;
	listShotPlayer_.push_back(obj);
}
shared_ptr<Stage_ObjShot> Stage_ShotManager::AddPlayerShot(CD3DXVECTOR2 pos, double speed, double angle,
	int graphic, size_t delay, IntersectPolarity polarity)
{
	shared_ptr<Stage_ObjShot> shot = this->CreateShotA1(ShotOwnerType::Player, pos, speed, angle, graphic, delay);
	shot->polarity_ = polarity;
	if (listShotPlayer_.size() < 1024)
		listShotPlayer_.push_back(shot);
	return shot;
}

void Stage_ShotManager::Render(byte layer) {
	for (auto& iShot : listShotPlayer_) {
		if (iShot->GetRenderPriority() == layer)
			iShot->Render();
	}
	for (auto& iShot : listShotEnemy_) {
		if (iShot->GetRenderPriority() == layer)
			iShot->Render();
	}

	{
		WindowMain* window = WindowMain::GetBase();
		IDirect3DDevice9* device = window->GetDevice();

		window->SetTextureFilter(D3DTEXF_LINEAR, D3DTEXF_LINEAR);

		VertexBufferManager* bufferManager = window->GetVertexManager();

		shaderLayer_->SetTechniqueByName("Render");
		ID3DXEffect* effect = shaderLayer_->GetEffect();
		{
			D3DXHANDLE handle = nullptr;
			if (handle = effect->GetParameterBySemantic(nullptr, "VIEWPROJECTION"))
				effect->SetMatrix(handle, window->GetViewportMatrix());
		}

		device->SetVertexDeclaration(bufferManager->GetDeclaration(1));
		device->SetFVF(VertexTLX::VertexFormat);

		for (auto itrSet = listShotRendererSet_.begin(); itrSet != listShotRendererSet_.end(); ++itrSet) {
			ListRenderer* renderBlendList = itrSet->second;

			device->SetTexture(0, itrSet->first->GetTexture());
			//device->SetTexture(0, nullptr);

			{
				UINT cPass = 1U;
				effect->Begin(&cPass, 0);
				if (cPass >= 1) {
					effect->BeginPass(0);
					for (auto itrBlend = renderBlendList->begin(); itrBlend != renderBlendList->end(); ++itrBlend) {
						if (itrBlend->second->countRenderIndex_ < 3) continue;

						window->SetBlendMode(itrBlend->first);
						itrBlend->second->Render();
					}
					effect->EndPass();
				}
				effect->End();
			}
		}

		device->SetVertexDeclaration(nullptr);
	}
}
void Stage_ShotManager::Update() {
	for (auto itr = listShotPlayer_.begin(); itr != listShotPlayer_.end(); ) {
		auto& iShot = *itr;
		if (iShot->IsDeleted()) itr = listShotPlayer_.erase(itr);
		else {
			(*itr)->ClearIntersected();
			(*itr)->Update();
			++itr;
		}
	}
	for (auto itr = listShotEnemy_.begin(); itr != listShotEnemy_.end(); ) {
		auto& iShot = *itr;
		if (iShot->IsDeleted()) itr = listShotEnemy_.erase(itr);
		else {
			(*itr)->ClearIntersected();
			(*itr)->Update();
			++itr;
		}
	}
	++frame_;
}

void Stage_ShotManager::DeleteInCircle(ShotOwnerType typeOwner, int64_t cx, int64_t cy, int64_t radius, bool bForce) {
	std::list<shared_ptr<Stage_ObjShot>>* pList = nullptr;
	if (typeOwner == ShotOwnerType::Player)
		pList = &listShotPlayer_;
	else
		pList = &listShotEnemy_;

	int64_t rr = radius * radius;
	for (auto itr = pList->begin(); itr != pList->end(); ++itr) {
		Stage_ObjShot* shot = itr->get();
		if (shot->IsDeleted() || shot->frameFadeDelete_ >= 0) continue;
		if (!bForce && shot->frameClipImmune_ > 0) continue;

		if (Math::HypotSq<int64_t>(cx - shot->posX_, cy - shot->posY_) <= rr) {
			pTaskDeleteEffect_->AddInstance(shot);
			shot->frameFadeDelete_ = Stage_ObjShot::FADE_MAX;
			//shot->SetDeleted(true);
		}
	}
}

//*******************************************************************
//Stage_ShotRenderer
//*******************************************************************
Stage_ShotRenderer::Stage_ShotRenderer() {
	countRenderVertex_ = 0U;
	countMaxVertex_ = 4096U;
	countRenderIndex_ = 0U;
	countMaxIndex_ = 8192U;

	SetVertexCount(countMaxVertex_);
	index_.resize(countMaxIndex_);
}

HRESULT Stage_ShotRenderer::Render() {
	//if (countRenderIndex_ < 3) return E_FAIL;

	WindowMain* window = WindowMain::GetBase();
	IDirect3DDevice9* device = window->GetDevice();

	VertexBufferManager* bufferManager = window->GetVertexManager();

	DxVertexBuffer* bufferVertex = bufferManager->GetDynamicVertexBufferTLX();
	DxIndexBuffer* bufferIndex = bufferManager->GetDynamicIndexBuffer();
	{
		BufferLockParameter lockParam = BufferLockParameter(D3DLOCK_DISCARD);
		lockParam.SetSource(vertex_, countRenderVertex_, sizeof(VertexTLX));
		HRESULT hr = bufferVertex->UpdateBuffer(&lockParam);
		if (FAILED(hr)) return hr;
	}
	{
		BufferLockParameter lockParam = BufferLockParameter(D3DLOCK_DISCARD);
		lockParam.SetSource(index_, countRenderIndex_, sizeof(uint16_t));
		HRESULT hr = bufferIndex->UpdateBuffer(&lockParam);
		if (FAILED(hr)) return hr;
	}

	device->SetStreamSource(0, bufferVertex->GetBuffer(), 0, sizeof(VertexTLX));
	device->SetIndices(bufferIndex->GetBuffer());

	device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, countRenderVertex_, 0, countRenderIndex_ / 3U);

	countRenderVertex_ = 0U;
	countRenderIndex_ = 0U;

	return S_OK;
}

bool Stage_ShotRenderer::_TryExpandVertex(size_t chk) {
	//Expands the vertex buffer if its size is insufficient for the next batch
	if (countMaxVertex_ >= DX_MAX_BUFFER_SIZE) return false;
	if (chk >= countMaxVertex_) {
		countMaxVertex_ = countMaxVertex_ << 1;
		SetVertexCount(countMaxVertex_);
	}
	return true;
}
bool Stage_ShotRenderer::_TryExpandIndex(size_t chk) {
	//Expands the index buffer if its size is insufficient for the next batch
	if (countMaxIndex_ >= DX_MAX_BUFFER_SIZE) return false;
	if (chk >= countMaxIndex_) {
		countMaxIndex_ = countMaxIndex_ << 1;
		index_.resize(countMaxIndex_);
	}
	return true;
}
void Stage_ShotRenderer::AddSquareVertex(VertexTLX* listVertex) {
	bool bNextAvailVertex = _TryExpandVertex(countRenderVertex_ + 4U);
	bool bNextAvailIndex = _TryExpandIndex(countRenderIndex_ + 6U);

	/*
	* Vertex arrangement:
	*     0------1
	*     |      |
	*     |      |
	*     2------3
	*/

	if (bNextAvailVertex && bNextAvailIndex) {
		memcpy(&vertex_[countRenderVertex_], listVertex, sizeof(VertexTLX) * 4U);

		index_[countRenderIndex_ + 0] = countRenderVertex_ + 0;
		index_[countRenderIndex_ + 1] = countRenderVertex_ + 1;
		index_[countRenderIndex_ + 2] = countRenderVertex_ + 2;
		index_[countRenderIndex_ + 3] = countRenderVertex_ + 2;
		index_[countRenderIndex_ + 4] = countRenderVertex_ + 1;
		index_[countRenderIndex_ + 5] = countRenderVertex_ + 3;

		countRenderVertex_ += 4U;
		countRenderIndex_ += 6U;
	}
}

//*******************************************************************
//Stage_ShotAnimation
//*******************************************************************
Stage_ShotAnimation::Stage_ShotAnimation(shared_ptr<TextureResource> texture, double spin, bool fixedAngle) {
	pDelayData_ = nullptr;

	texture_ = texture;
	width_ = texture_->GetWidth();
	height_ = texture_->GetHeight();

	spin_ = Math::DegreeToRadian(spin);
	bFixedAngle_ = fixedAngle;

	hitCircle_ = DxCircle<float>(0, 0, -1);
}

const Stage_ShotAnimation::Frame* Stage_ShotAnimation::GetFrame(size_t frame) const {
	if (listFrameData_.size() == 1U)
		return &listFrameData_[0];

	frame %= maxFrame_;
	for (auto& iFrame : listFrameData_) {
		if (frame < iFrame.frame)
			return &iFrame;
		frame -= iFrame.frame;
	}

	return nullptr;
}

Stage_ShotRenderer* Stage_ShotAnimation::GetRendererFromBlendType(BlendMode blend) {
	if (pAttachedRenderer_ == nullptr) return nullptr;
	auto itrFind = pAttachedRenderer_->find(blend);
	if (itrFind != pAttachedRenderer_->end())
		return itrFind->second;
	return nullptr;
}

//*******************************************************************
//Stage_ObjShot
//*******************************************************************
Stage_ObjShot::Stage_ObjShot(Stage_ShotManager* manager) {
	shotManager_ = manager;

	SetRenderPriority(LAYER_SHOT);

	polarity_ = IntersectPolarity::White;
	typeOwner_ = ShotOwnerType::Enemy;

	frame_ = 0;
	frameDelay_ = 0;
	frameClipImmune_ = 0;
	frameFadeDelete_ = -1;

	pShotData_ = nullptr;
	shotAngleZ_ = 0;

	life_ = 1;
	damage_ = 1;

	vertex_.resize(4U);
}
Stage_ObjShot::~Stage_ObjShot() {
}

void Stage_ObjShot::_DeleteInLife() {
	if (IsDeleted()) return;
	if (life_ <= 0) {
		SetDeleted(true);
	}
}
void Stage_ObjShot::_DeleteInAutoClip() {
	if (IsDeleted()) return;
	if (frameClipImmune_ == 0) {
		const DxRectangle<int>* clip = shotManager_->GetClip();

		if (posX_ < clip->left || posX_ > clip->right || posY_ < clip->top || posY_ > clip->bottom)
			SetDeleted(true);
	}
	else --frameClipImmune_;
}
void Stage_ObjShot::_DeleteInFade() {
	if (IsDeleted()) return;
	if (frameFadeDelete_ == 0) {
		SetDeleted(true);
	}
	else --frameFadeDelete_;
}
void Stage_ObjShot::_CommonUpdate() {
	_DeleteInLife();
	_DeleteInAutoClip();
	_DeleteInFade();

	++frame_;
}

void Stage_ObjShot::SetShotData(int id) {
	if (typeOwner_ == ShotOwnerType::Player)
		pShotData_ = shotManager_->GetPlayerShotData(id);
	else
		pShotData_ = shotManager_->GetEnemyShotData(id);
}
void Stage_ObjShot::SetShotData(Stage_ShotAnimation* id) {
	pShotData_ = id;
}

void Stage_ObjShot::_LoadVertices(const DxRectangle<float>* rcSrc, const DxRectangle<float>* rcDst, 
	D3DCOLOR color, float scale, CD3DXVECTOR2 pos)
{
	float scx = scale_.x * scale;
	float scy = scale_.y * scale;

	VertexTLX* pVertex = vertex_.data();
	SetSourceRectNormalized(pVertex, rcSrc);
	SetDestRect(pVertex, rcDst);
	for (int i = 0; i < 4; ++i) {
		float vx = pVertex[i].position.x * scx;
		float vy = pVertex[i].position.y * scy;
		pVertex[i].position.x = (vx * angleZ_.x - vy * angleZ_.y) + pos.x;
		pVertex[i].position.y = (vx * angleZ_.y + vy * angleZ_.x) + pos.y;
	}
	SetColor(pVertex, color);
}
HRESULT Stage_ObjShot::Render() {
	if (IsDeleted()) return E_FAIL;
	if (pShotData_ == nullptr) return D3DERR_INVALIDCALL;

	Stage_ShotRenderer* renderer = nullptr;

	const DxRectangle<float>* rcSrc = nullptr;
	const DxRectangle<float>* rcDst = nullptr;

	float scale = 1.0f;
	int alpha = 255;

	if (frameDelay_ > 0) {
		Stage_ShotAnimation* dataDelay = pShotData_->GetDelayData();
		if (dataDelay == nullptr) return D3DERR_INVALIDCALL;

		renderer = dataDelay->GetRendererFromBlendType(blend_);
		if (renderer == nullptr) return D3DERR_INVALIDCALL;

		const Stage_ShotAnimation::Frame* animFrameDelay = dataDelay->GetFrame(0);
		rcSrc = &animFrameDelay->rcSrc;
		rcDst = &animFrameDelay->rcDst;

		float avgShotSize = (pShotData_->GetWidth() + pShotData_->GetHeight()) / 2.0f;

		float delayMul = frameDelay_ / (float)frameDelayMax_;
		scale = Math::Lerp::Smooth(1.0f, 2.5f, delayMul) * (avgShotSize / 64.0f);
		alpha = Math::Lerp::Linear(256 + 128, 0, delayMul);
	}
	else {
		renderer = pShotData_->GetRendererFromBlendType(blend_);
		if (renderer == nullptr) return D3DERR_INVALIDCALL;

		const Stage_ShotAnimation::Frame* animFrame = pShotData_->GetFrame(frame_);
		if (animFrame == nullptr) return D3DERR_INVALIDCALL;

		rcSrc = &animFrame->rcSrc;
		rcDst = &animFrame->rcDst;

		if (frameFadeDelete_ >= 0) {
			float fadeMul = frameFadeDelete_ / (float)FADE_MAX;
			scale = Math::Lerp::Smooth(2.3f, 1.0f, fadeMul);
			alpha = Math::Lerp::Linear(0, 255, fadeMul);
		}
		else {
			scale = 1.0f;
			alpha = 255;
		}
	}

	D3DXVECTOR4 colorCopy = color_;
	colorCopy.w *= ColorUtility::ClampRet(alpha) / 255.0f;

	D3DCOLOR color = ColorUtility::VectorToD3DColor(colorCopy);
	//if (IsIntersected())
	//	color = (D3DCOLOR_XRGB(255, 16, 64) & 0x00ffffff) | (color & 0xff000000);

	FLOAT sposx = position_.x - 0.5f;
	FLOAT sposy = position_.y - 0.5f;

	_LoadVertices(rcSrc, rcDst, color, scale, D3DXVECTOR2(sposx, sposy));
	renderer->AddSquareVertex(vertex_.data());

	return S_OK;
}
void Stage_ObjShot::Update() {
	ClearIntersected();
	ClearIntersectedList();

	if (frameFadeDelete_ < 0) {
		_Move();
		RenderObject::SetPosition(posX_, posY_, 1.0f);

		if (frameDelay_ > 0) --frameDelay_;
		{
			double angleZ = shotAngleZ_;
			if (pShotData_ && !pShotData_->bFixedAngle_) {
				angleZ += GetDirectionAngle() + Math::DegreeToRadian(90);
				shotAngleZ_ += pShotData_->spin_;
			}
			SetAngleZ(angleZ);
		}

		_RegistIntersection();
	}
	_CommonUpdate();
}

void Stage_ObjShot::SetSourceRectNormalized(VertexTLX* vert, const DxRectangle<float>* rc) {
	vert[0].texcoord = D3DXVECTOR2(rc->left, rc->top);
	vert[1].texcoord = D3DXVECTOR2(rc->right, rc->top);
	vert[2].texcoord = D3DXVECTOR2(rc->left, rc->bottom);
	vert[3].texcoord = D3DXVECTOR2(rc->right, rc->bottom);
}
void Stage_ObjShot::SetDestRect(VertexTLX* vert, const DxRectangle<float>* rc) {
	vert[0].position = D3DXVECTOR3(rc->left, rc->top, 1.0f);
	vert[1].position = D3DXVECTOR3(rc->right, rc->top, 1.0f);
	vert[2].position = D3DXVECTOR3(rc->left, rc->bottom, 1.0f);
	vert[3].position = D3DXVECTOR3(rc->right, rc->bottom, 1.0f);
}
void Stage_ObjShot::SetColor(VertexTLX* vert, D3DCOLOR color) {
	vert[0].diffuse = color;
	vert[1].diffuse = color;
	vert[2].diffuse = color;
	vert[3].diffuse = color;
}

void Stage_ObjShot::_RegistIntersection() {
	if (IsDeleted() || frameDelay_ > 0 || frameFadeDelete_ >= 0) return;
	if (pShotData_ == nullptr) return;

	const DxCircle<float>* pBaseCircle = pShotData_->GetHitboxCircle();
	if (pBaseCircle->r > 0) {
		if (pIntersectionTarget_ == nullptr) {
			pIntersectionTarget_ = shared_ptr<Stage_IntersectionTarget_Circle>(new Stage_IntersectionTarget_Circle());
			pIntersectionTarget_->SetTargetType(typeOwner_ == ShotOwnerType::Player?
				Stage_IntersectionTarget::TypeTarget::PlayerShot : Stage_IntersectionTarget::TypeTarget::EnemyShot);
			pIntersectionTarget_->SetParent(pOwnRefWeak_);
		}

		Stage_IntersectionTarget_Circle* pTargetCircle = (Stage_IntersectionTarget_Circle*)pIntersectionTarget_.get();

		auto& targetCircle = pTargetCircle->GetCircle();
		targetCircle.x = posX_ + pBaseCircle->x;
		targetCircle.y = posY_ + pBaseCircle->y;
		targetCircle.r = pBaseCircle->r;
		pTargetCircle->SetIntersectionSpace();

		auto intersectionManager = ((Stage_MainScene*)shotManager_->GetParent())->GetIntersectionManager();
		intersectionManager->AddTarget(pIntersectionTarget_->GetTargetType(), pIntersectionTarget_);
	}
}

void Stage_ObjShot::Intersect(shared_ptr<Stage_IntersectionTarget> ownTarget, shared_ptr<Stage_IntersectionTarget> otherTarget) {
	shared_ptr<Stage_ObjCollision> pOther = otherTarget->GetParent().lock();

	switch (otherTarget->GetTargetType()) {
	case Stage_IntersectionTarget::TypeTarget::Player:
		if (auto pTarget = dynamic_cast<Stage_IntersectionTarget_Player*>(otherTarget.get())) {
			auto objPlayer = dynamic_cast<Stage_PlayerTask*>(pTarget->GetParent().lock().get());
			if (pTarget->IsGraze()) {
				if (polarity_ == objPlayer->GetPolarity())
					frameFadeDelete_ = FADE_MAX;
			}
			else {
				if (polarity_ != objPlayer->GetPolarity())
					life_ = 0;
			}
		}
		break;
	case Stage_IntersectionTarget::TypeTarget::PlayerSpell:
		break;
	case Stage_IntersectionTarget::TypeTarget::EnemyToPlayerShot:
		life_ -= 1;
		break;
	}
}

//*******************************************************************
//Stage_ShotManager
//*******************************************************************
Stage_ShotDeleteEffectRendererTask::Stage_ShotDeleteEffectRendererTask(Scene* parent) : TaskBase(parent) {
	IDirect3DDevice9* device = WindowMain::GetBase()->GetDevice();

	GET_INSTANCE(ResourceManager, resourceManager);
	texture_ = resourceManager->GetResourceAs<TextureResource>("img/stage/eff_shotbreak.png");
	objEffects_.SetTexture(texture_);
	objEffects_.SetShader(resourceManager->GetResourceAs<ShaderResource>("shader/instanced_shotbreak.fx"));

	objEffects_.SetSourceRect(DxRectangle<int>(0, 0, 32, 32));
	objEffects_.SetDestCenter();
	{
		D3DCOLOR color = D3DCOLOR_ARGB(160, 255, 255, 255);
		for (int i = 0; i < 4; ++i) {
			objEffects_.GetVertex(i)->diffuse = color;
		}
	}
	objEffects_.UpdateVertexBuffer();

	{
		std::vector<uint16_t> index = { 0, 1, 2, 3 };

		objEffects_.SetArrayIndex(index);
	}
}

void Stage_ShotDeleteEffectRendererTask::Render(byte layer) {
	if (layer != LAYER_SHOT - 1) return;
	objEffects_.Render();
}
void Stage_ShotDeleteEffectRendererTask::Update() {
	objEffects_.ClearInstance();
	for (auto itr = listParticle_.begin(); itr != listParticle_.end();) {
		ParticleData*& pData = (*itr);
		if (pData->life >= FRAME_PER_STEP * 8) {
			ptr_delete(pData);
			itr = listParticle_.erase(itr);
		}
		else {
			pData->pos += pData->move;

			size_t u_off = (pData->life / FRAME_PER_STEP) * 32;
			{
				InstanceData iInst;
				iInst.diffuse_color = pData->color;
				iInst.vec4a = D3DXVECTOR4(pData->pos.x, pData->pos.y, pData->scale, pData->scale);
				iInst.vec4b = D3DXVECTOR4(pData->angle.x, pData->angle.y, u_off / (float)texture_->GetWidth(), 0);
				objEffects_.AddInstance(iInst);
			}

			++(pData->life);
			++itr;
		}
	}
}

void Stage_ShotDeleteEffectRendererTask::AddInstance(Stage_ObjShot* shot) {
	if (listParticle_.size() == 8192) return;

	auto movePattern = shot->GetPattern().get();
	auto shotData = shot->GetShotData();

	ParticleData* newData = new ParticleData();
	newData->pos = shot->GetMovePosition();
	newData->move = D3DXVECTOR2(0, 0);
	if (movePattern) {
		newData->move = D3DXVECTOR2(movePattern->GetSpeedX(), movePattern->GetSpeedY());
	}
	{
		float zAng = RandProvider::GetBase()->GetReal(0, GM_PI_X2);
		newData->angle = D3DXVECTOR2(sinf(zAng), cosf(zAng));
	}
	newData->color = 0xffffffff;
	newData->scale = 1;
	if (shotData) {
		constexpr float breakSize = 20;
		newData->color = shotData->GetColor();
		newData->scale = breakSize / ((shotData->GetWidth() + shotData->GetHeight()) / 2.0f);
	}
	newData->life = 0;
	listParticle_.push_back(newData);
}