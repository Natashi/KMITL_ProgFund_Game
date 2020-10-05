#include "pch.h"
#include "../Engine/Window.hpp"
#include "Object.hpp"

//*******************************************************************
//ObjectBase
//*******************************************************************
ObjectBase::ObjectBase() {
	type_ = TypeObject::Null;
	renderPri_ = 40;
	bVisible_ = true;
	bDeleted_ = false;
}
ObjectBase::~ObjectBase() {
}

//*******************************************************************
//RenderObject
//*******************************************************************
RenderObject::RenderObject() {
	SetType(TypeObject::Render);

	position_ = D3DXVECTOR3(0, 0, 0);
	angle_ = D3DXVECTOR3(0, 0, 0);
	angleX_ = D3DXVECTOR2(1, 0);
	angleY_ = D3DXVECTOR2(1, 0);
	angleZ_ = D3DXVECTOR2(1, 0);
	scale_ = D3DXVECTOR3(1, 1, 1);
	color_ = D3DXVECTOR4(1, 1, 1, 1);

	primitiveType_ = D3DPT_TRIANGLELIST;
	blend_ = BlendMode::Alpha;

	SetTexture(nullptr);
	SetShader(nullptr);
}
RenderObject::~RenderObject() {
}

D3DXMATRIX RenderObject::CreateWorldMatrix2D(D3DXVECTOR3* const position, D3DXVECTOR3* const angle,
	D3DXVECTOR3* const scale, D3DXMATRIX* const camera)
{
	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);
	if (scale->x != 1.0f || scale->y != 1.0f || scale->z != 1.0f) {
		D3DXMatrixScaling(&mat, scale->x, scale->y, scale->z);
	}
	if (angle->x != 0.0f || angle->y != 0.0f || angle->z != 0.0f) {
		D3DXMATRIX matRot;
		D3DXMatrixRotationYawPitchRoll(&matRot, angle->y, angle->x, angle->z);
		D3DXMatrixMultiply(&mat, &mat, &matRot);
	}
	if (position->x != 0.0f || position->y != 0.0f || position->z != 0.0f) {
		mat._41 = position->x;
		mat._42 = position->y;
		mat._43 = position->z;
	}
	if (camera) D3DXMatrixMultiply(&mat, &mat, camera);
	return mat;
}
D3DXMATRIX RenderObject::CreateWorldMatrix2D(D3DXVECTOR3* const position, D3DXVECTOR2* const angleX,
	D3DXVECTOR2* const angleY, D3DXVECTOR2* const angleZ, D3DXVECTOR3* const scale, D3DXMATRIX* const camera)
{
	D3DXMATRIX mat;
	D3DXMatrixIdentity(&mat);
	if (scale->x != 1.0f || scale->y != 1.0f || scale->z != 1.0f) {
		D3DXMatrixScaling(&mat, scale->x, scale->y, scale->z);
	}
	if (angleZ->x != 1.0f || angleZ->y != 0.0f || angleX->x != 1.0f || angleX->y != 0.0f 
		|| angleY->x != 1.0f || angleY->y != 0.0f)
	{
		D3DXMATRIX matRot;

		float cx = angleX->x;
		float sx = angleX->y;
		float cy = angleY->x;
		float sy = angleY->y;
		float cz = angleZ->x;
		float sz = angleZ->y;
		float sx_sy = sx * sy;
		float sx_cy = sx * cy;

#ifdef __L_MATH_VECTORIZE
		__m128 v1 = Vectorize::Mul(Vectorize::Set(cy, sx_sy, cx, sy), Vectorize::Set(cz, sz, sz, cz));
		__m128 v2 = Vectorize::Mul(Vectorize::Set(sx_cy, cy, sx_sy, cx), Vectorize::Set(sz, sz, cz, cz));
		__m128 v3 = Vectorize::Mul(Vectorize::Set(sy, sx_cy, cx, cx), Vectorize::Set(sz, cz, sy, cy));

		matRot._12 = -v1.m128_f32[2];
		matRot._22 = v2.m128_f32[3];
		matRot._31 = -v3.m128_f32[2];
		matRot._32 = sx;
		matRot._33 = v3.m128_f32[3];

		v1 = Vectorize::AddSub(
			Vectorize::Set(v1.m128_f32[0], v1.m128_f32[3], v3.m128_f32[0], v2.m128_f32[1]),
			Vectorize::Set(v1.m128_f32[1], v2.m128_f32[0], v3.m128_f32[1], v2.m128_f32[2]));
		matRot._11 = v1.m128_f32[0];
		matRot._13 = v1.m128_f32[1];
		matRot._21 = v1.m128_f32[3];
		matRot._23 = v1.m128_f32[2];
#else
		matRot._11 = cy * cz - sx_sy * sz;
		matRot._12 = -cx * sz;
		matRot._13 = sy * cz + sx_cy * sz;
		matRot._21 = cy * sz + sx_sy * cz;
		matRot._22 = cx * cz;
		matRot._23 = sy * sz - sx_cy * cz;
		matRot._31 = -cx * sy;
		matRot._32 = sx;
		matRot._33 = cx * cy;
#endif

		D3DXMatrixMultiply(&mat, &mat, &matRot);
	}
	if (position->x != 0.0f || position->y != 0.0f || position->z != 0.0f) {
		mat._41 = position->x;
		mat._42 = position->y;
		mat._43 = position->z;
	}
	if (camera) D3DXMatrixMultiply(&mat, &mat, camera);
	return mat;
}
size_t RenderObject::GetPrimitiveCount(D3DPRIMITIVETYPE type, size_t count) {
	switch (type) {
	case D3DPT_POINTLIST:
		return count;
	case D3DPT_LINELIST:
		return count / 2U;
	case D3DPT_LINESTRIP:
		return (count > 0U ? count - 1U : 0U);
	case D3DPT_TRIANGLELIST:
		return count / 3U;
	case D3DPT_TRIANGLESTRIP:
	case D3DPT_TRIANGLEFAN:
		return (count > 1U ? count - 2U : 0U);
	}
	return 0U;
}
size_t RenderObject::GetPrimitiveCount() {
	return GetPrimitiveCount(primitiveType_, index_.size() > 0 ? index_.size() : vertex_.size());
}

void RenderObject::SetAngleX(float x) {
	if (angle_.x != x) {
		angle_.x = x;
		angleX_ = D3DXVECTOR2(cosf(x), sinf(x));
	}
}
void RenderObject::SetAngleY(float y) {
	if (angle_.y != y) {
		angle_.y = y;
		angleY_ = D3DXVECTOR2(cosf(y), sinf(y));
	}
}
void RenderObject::SetAngleZ(float z) {
	if (angle_.z != z) {
		angle_.z = z;
		angleZ_ = D3DXVECTOR2(cosf(z), sinf(z));
	}
}

void RenderObject::SetVertex(size_t index, const VertexTLX& vertex) {
	VertexTLX* dst = &vertex_[index];
	memcpy(dst, &vertex, sizeof(VertexTLX));
}
VertexTLX* RenderObject::GetVertex(size_t index) {
	return &vertex_[index];
}

//*******************************************************************
//StaticRenderObject
//*******************************************************************
StaticRenderObject::StaticRenderObject() {
	scroll_ = D3DXVECTOR2(0, 0);

	Initialize();
}
StaticRenderObject::~StaticRenderObject() {
}

void StaticRenderObject::Initialize() {
	IDirect3DDevice9* device = WindowMain::GetBase()->GetDevice();
	bufferVertex_ = std::shared_ptr<DxVertexBuffer>(new DxVertexBuffer(device, 0));
	bufferIndex_ = std::shared_ptr<DxIndexBuffer>(new DxIndexBuffer(device, 0));
	bufferVertex_->Create(64U, sizeof(VertexTLX), D3DPOOL_MANAGED, VertexTLX::VertexFormat);
}
void StaticRenderObject::Update() {
}
HRESULT StaticRenderObject::Render() {
	if (shader_ == nullptr) return D3DERR_INVALIDCALL;

	WindowMain* window = WindowMain::GetBase();
	IDirect3DDevice9* device = WindowMain::GetBase()->GetDevice();

	window->SetTextureFilter(D3DTEXF_LINEAR, D3DTEXF_LINEAR);
	window->SetBlendMode(blend_);

	D3DXMATRIX matWorld = RenderObject::CreateWorldMatrix2D(&position_, &angleX_, &angleY_,
		&angleZ_, &scale_, nullptr);

	ID3DXEffect* effect = shader_->GetEffect();
	{
		D3DXHANDLE handle = nullptr;
		if (handle = effect->GetParameterBySemantic(nullptr, "WORLD"))
			effect->SetMatrix(handle, &matWorld);
		if (handle = effect->GetParameterBySemantic(nullptr, "VIEWPROJECTION"))
			effect->SetMatrix(handle, window->GetViewportMatrix());
		if (handle = effect->GetParameterBySemantic(nullptr, "OBJCOLOR"))
			effect->SetVector(handle, &color_);
		if (handle = effect->GetParameterBySemantic(nullptr, "UVSCROLL"))
			effect->SetFloatArray(handle, (float*)&scroll_, 2U);
	}

	shader_->SetTechnique("Render");

	device->SetTexture(0, texture_->GetTexture());
	device->SetFVF(VertexTLX::VertexFormat);

	device->SetVertexDeclaration(VertexDeclarationManager::GetBase()->GetDeclarationTLX());
	device->SetStreamSource(0, bufferVertex_->GetBuffer(), 0, sizeof(VertexTLX));

	{
		size_t countPrim = GetPrimitiveCount();
		bool bIndex = index_.size() > 0;
		if (bIndex) device->SetIndices(bufferIndex_->GetBuffer());

		UINT countPass = 1;
		HRESULT hr = effect->Begin(&countPass, 0);
		if (FAILED(hr)) return hr;
		for (UINT iPass = 0; iPass < countPass; ++iPass) {
			effect->BeginPass(iPass);

			if (bIndex)
				device->DrawIndexedPrimitive(primitiveType_, 0, 0, vertex_.size(), 0, countPrim);
			else
				device->DrawPrimitive(primitiveType_, 0, countPrim);

			effect->EndPass();
		}
		effect->End();
	}

	device->SetVertexDeclaration(nullptr);

	return S_OK;
}

void StaticRenderObject::UpdateVertexBuffer() {
	size_t sizeBuffer = bufferVertex_->GetSize();
	if (vertex_.size() > sizeBuffer) {
		while (vertex_.size() > sizeBuffer)
			sizeBuffer = sizeBuffer << 1;
		bufferVertex_->Create(sizeBuffer, sizeof(VertexTLX), D3DPOOL_MANAGED, VertexTLX::VertexFormat);
	}
	BufferLockParameter lockParam = BufferLockParameter(D3DLOCK_DISCARD);
	lockParam.SetSource(vertex_, 0x8000u, sizeof(VertexTLX));
	bufferVertex_->UpdateBuffer(&lockParam);
}
void StaticRenderObject::UpdateIndexBuffer() {
	size_t sizeBuffer = bufferIndex_->GetSize();
	if (index_.size() > sizeBuffer) {
		while (index_.size() > sizeBuffer)
			sizeBuffer = sizeBuffer << 1;
		bufferIndex_->Create(sizeBuffer, sizeof(uint16_t), D3DPOOL_MANAGED, D3DFMT_INDEX16);
	}
	BufferLockParameter lockParam = BufferLockParameter(D3DLOCK_DISCARD);
	lockParam.SetSource(index_, 0x8000u, sizeof(uint16_t));
	bufferIndex_->UpdateBuffer(&lockParam);
}
void StaticRenderObject::SetArrayVertex(const std::vector<VertexTLX>& vertices) {
	vertex_ = std::vector<VertexTLX>(vertices.begin(),
		std::min(vertices.begin() + 0x8000u, vertices.end()));
	UpdateVertexBuffer();
}
void StaticRenderObject::SetArrayIndex(const std::vector<uint16_t>& indices) {
	index_ = std::vector<uint16_t>(indices.begin(),
		std::min(indices.begin() + 0x8000u, indices.end()));
	UpdateIndexBuffer();
}

//*******************************************************************
//Sprite2D
//*******************************************************************
Sprite2D::Sprite2D() {
	primitiveType_ = D3DPT_TRIANGLESTRIP;

	bPermitCamera_ = true;

	Initialize();
}
Sprite2D::~Sprite2D() {
}

void Sprite2D::Initialize() {
	IDirect3DDevice9* device = WindowMain::GetBase()->GetDevice();
	bufferVertex_ = std::shared_ptr<DxVertexBuffer>(new DxVertexBuffer(device, 0));
	bufferIndex_ = std::shared_ptr<DxIndexBuffer>(new DxIndexBuffer(device, 0));
	bufferVertex_->Create(4U, sizeof(VertexTLX), D3DPOOL_MANAGED, VertexTLX::VertexFormat);
	vertex_.resize(4U, VertexTLX());
}

void Sprite2D::SetSourceRectNormalized(const DxRect& rc) {
	GetVertex(0)->texcoord = D3DXVECTOR2(rc.left, rc.top);
	GetVertex(1)->texcoord = D3DXVECTOR2(rc.right, rc.top);
	GetVertex(2)->texcoord = D3DXVECTOR2(rc.left, rc.bottom);
	GetVertex(3)->texcoord = D3DXVECTOR2(rc.right, rc.bottom);

	//UpdateVertexBuffer();
}
void Sprite2D::SetSourceRect(const DxRect& rc) {
	float width = texture_->GetImageInfo()->Width;
	float height = texture_->GetImageInfo()->Height;

	GetVertex(0)->texcoord = D3DXVECTOR2(rc.left / width, rc.top / height);
	GetVertex(1)->texcoord = D3DXVECTOR2(rc.right / width, rc.top / height);
	GetVertex(2)->texcoord = D3DXVECTOR2(rc.left / width, rc.bottom / height);
	GetVertex(3)->texcoord = D3DXVECTOR2(rc.right / width, rc.bottom / height);

	//UpdateVertexBuffer();
}
void Sprite2D::SetDestRect(const DxRect& rc) {
	GetVertex(0)->position = D3DXVECTOR3(rc.left, rc.top, 1.0f);
	GetVertex(1)->position = D3DXVECTOR3(rc.right, rc.top, 1.0f);
	GetVertex(2)->position = D3DXVECTOR3(rc.left, rc.bottom, 1.0f);
	GetVertex(3)->position = D3DXVECTOR3(rc.right, rc.bottom, 1.0f);
	for (int i = 0; i < 4; ++i)
		GetVertex(i)->Bias(-0.5f);

	//UpdateVertexBuffer();
}
void Sprite2D::SetDestCenter() {
	VertexTLX* vertTL = GetVertex(0);
	VertexTLX* vertBR = GetVertex(3);

	float width = texture_->GetImageInfo()->Width;
	float height = texture_->GetImageInfo()->Height;
	float vWidth = (vertBR->texcoord.x - vertTL->texcoord.x) * width / 2.0f;
	float vHeight = (vertBR->texcoord.y - vertTL->texcoord.y) * height / 2.0f;
	SetDestRect(DxRect(-vWidth, -vHeight, vWidth, vHeight));

	//UpdateVertexBuffer();
}