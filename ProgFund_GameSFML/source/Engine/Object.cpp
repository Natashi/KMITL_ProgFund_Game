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

XMMATRIX RenderObject::CreateWorldMatrix2D(D3DXVECTOR3* const position, D3DXVECTOR3* const angle,
	D3DXVECTOR3* const scale, XMMATRIX* const camera)
{
	XMMATRIX mat = XMMatrixIdentity();
	if (scale->x != 1.0f || scale->y != 1.0f || scale->z != 1.0f) {
		mat = XMMatrixMultiply(mat, XMMatrixScaling(scale->x, scale->y, scale->z));
	}
	if (angle->x != 0.0f || angle->y != 0.0f || angle->z != 0.0f) {
		XMVECTOR vecSin, vecCos;
		XMVectorSinCos(&vecSin, &vecCos, XMVectorSet(angle->x, angle->y, angle->z, 0.0f));
		float cx = vecCos.m128_f32[0];
		float sx = vecSin.m128_f32[0];
		float cy = vecCos.m128_f32[1];
		float sy = vecSin.m128_f32[1];
		float cz = vecCos.m128_f32[2];
		float sz = vecSin.m128_f32[2];
		float sx_sy = sx * sy;
		float sx_cy = sx * cy;

		XMVECTOR vecA = XMVectorMultiply(XMVectorSet(cy, sx_sy, cx, sy), XMVectorSet(cz, sz, sz, cz));
		XMVECTOR vecB = XMVectorMultiply(XMVectorSet(sx_cy, cy, sx_sy, cx), XMVectorSet(sz, sz, cz, cz));
		XMVECTOR vecC = XMVectorMultiply(XMVectorSet(sy, sx_cy, cx, cx), XMVectorSet(sz, cz, sy, cy));
		float* _a = vecA.m128_f32;
		float* _b = vecB.m128_f32;
		float* _c = vecC.m128_f32;

		XMMATRIX matRotation;
		matRotation.r[0] = XMVectorSet(_a[0] - _a[1], -_a[2], _a[3] + _b[0], 0.0f);
		matRotation.r[1] = XMVectorSet(_b[1] + _b[2], _b[3], _c[0] - _c[1], 0.0f);
		matRotation.r[2] = XMVectorSet(-_c[2], vecCos.m128_f32[0], _c[3], 0.0f);
		matRotation.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		mat = XMMatrixMultiply(mat, matRotation);
	}
	if (position->x != 0.0f || position->y != 0.0f || position->z != 0.0f) {
		//mat = XMMatrixMultiply(mat, XMMatrixTranslation(position->x, position->y, position->z));
		mat.r[3] = XMVectorSet(position->x, position->y, position->z, 1.0f);
	}
	if (camera) mat = XMMatrixMultiply(mat, *camera);
	return mat;
}
XMMATRIX RenderObject::CreateWorldMatrix2D(D3DXVECTOR3* const position, D3DXVECTOR2* const angleX,
	D3DXVECTOR2* const angleY, D3DXVECTOR2* const angleZ, D3DXVECTOR3* const scale, XMMATRIX* const camera)
{
	XMMATRIX mat = XMMatrixIdentity();
	if (scale->x != 1.0f || scale->y != 1.0f || scale->z != 1.0f) {
		mat = XMMatrixMultiply(mat, XMMatrixScaling(scale->x, scale->y, scale->z));
	}
	{
		float cx = angleX->x;
		float sx = angleX->y;
		float cy = angleY->x;
		float sy = angleY->y;
		float cz = angleZ->x;
		float sz = angleZ->y;
		float sx_sy = sx * sy;
		float sx_cy = sx * cy;

		XMVECTOR vecA = XMVectorMultiply(XMVectorSet(cy, sx_sy, cx, sy), XMVectorSet(cz, sz, sz, cz));
		XMVECTOR vecB = XMVectorMultiply(XMVectorSet(sx_cy, cy, sx_sy, cx), XMVectorSet(sz, sz, cz, cz));
		XMVECTOR vecC = XMVectorMultiply(XMVectorSet(sy, sx_cy, cx, cx), XMVectorSet(sz, cz, sy, cy));
		float* _a = vecA.m128_f32;
		float* _b = vecB.m128_f32;
		float* _c = vecC.m128_f32;

		XMMATRIX matRotation;
		matRotation.r[0] = XMVectorSet(_a[0] - _a[1], -_a[2], _a[3] + _b[0], 0.0f);
		matRotation.r[1] = XMVectorSet(_b[1] + _b[2], _b[3], _c[0] - _c[1], 0.0f);
		matRotation.r[2] = XMVectorSet(-_c[2], angleX->y, _c[3], 0.0f);
		matRotation.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		mat = XMMatrixMultiply(mat, matRotation);
	}
	if (position->x != 0.0f || position->y != 0.0f || position->z != 0.0f) {
		//mat = XMMatrixMultiply(mat, XMMatrixTranslation(position->x, position->y, position->z));
		mat.r[3] = XMVectorSet(position->x, position->y, position->z, 1.0f);
	}
	if (camera) mat = XMMatrixMultiply(mat, *camera);
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
void StaticRenderObject::Render() {
	if (shader_ == nullptr) return;

	WindowMain* window = WindowMain::GetBase();
	IDirect3DDevice9* device = WindowMain::GetBase()->GetDevice();

	window->SetTextureFilter(D3DTEXF_LINEAR, D3DTEXF_LINEAR);
	window->SetBlendMode(blend_);

	XMMATRIX matWorld = RenderObject::CreateWorldMatrix2D(&position_, &angleX_, &angleY_,
		&angleZ_, &scale_, window->GetProjectionMatrix());

	ID3DXEffect* effect = shader_->GetEffect();
	{
		D3DXHANDLE handle = nullptr;
		if (handle = effect->GetParameterBySemantic(nullptr, "WORLD"))
			effect->SetMatrix(handle, (D3DXMATRIX*)&matWorld);
		if (handle = effect->GetParameterBySemantic(nullptr, "VIEWPROJECTION"))
			effect->SetMatrix(handle, (D3DXMATRIX*)window->GetProjectionMatrix());
		if (handle = effect->GetParameterBySemantic(nullptr, "OBJCOLOR"))
			effect->SetVector(handle, &color_);
	}

	shader_->SetTechnique("Render");

	device->SetVertexDeclaration(VertexDeclarationManager::GetBase()->GetDeclarationTLX());
	device->SetStreamSource(0, bufferVertex_->GetBuffer(), 0, sizeof(VertexTLX));

	{
		size_t countPrim = GetPrimitiveCount();
		bool bIndex = index_.size() > 0;
		if (bIndex) device->SetIndices(bufferIndex_->GetBuffer());

		UINT countPass = 1;
		effect->Begin(&countPass, 0);
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