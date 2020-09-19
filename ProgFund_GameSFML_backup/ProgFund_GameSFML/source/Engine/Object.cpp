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

	position_ = Vector3f(0, 0, 0);
	angle_ = Vector3f(0, 0, 0);
	angleX_ = Vector2f(1, 0);
	angleY_ = Vector2f(1, 0);
	angleZ_ = Vector2f(1, 0);
	scale_ = Vector3f(1, 1, 1);
	color_ = 0xffffffff;

	primitiveType_ = PrimitiveType::TriangleList;
	blend_ = BlendMode::Alpha;

	SetTexture(nullptr);
	SetShader(nullptr);
}
RenderObject::~RenderObject() {
}

XMMATRIX RenderObject::CreateWorldMatrix2D(Vector3f* const position, Vector3f* const angle,
	Vector3f* const scale, XMMATRIX* const camera)
{
	XMMATRIX mat = XMMatrixIdentity();
	if (scale->x != 1.0f || scale->y != 1.0f || scale->z != 1.0f) {
		mat = XMMatrixMultiply(mat, XMMatrixScaling(scale->x, scale->y, scale->z));
	}
	if (angle->x != 0.0f || angle->y != 0.0f || angle->z != 0.0f) {
		XMVECTOR vecSin, vecCos;
		XMVectorSinCos(&vecSin, &vecCos, XMVectorSet(angle->x, angle->y, angle->z, 0.0f));

		float sx_sy = vecCos.m128_f32[0] * vecCos.m128_f32[1];
		float sx_cy = vecCos.m128_f32[0] * vecSin.m128_f32[1];
		XMVECTOR vecSC1 = XMVectorSet(vecSin.m128_f32[0], vecCos.m128_f32[0], sx_sy, sx_cy);
		XMVECTOR vecSC2 = XMVectorSet(vecSin.m128_f32[1], vecCos.m128_f32[1], vecSin.m128_f32[2], vecCos.m128_f32[2]);

		constexpr const XMVECTORI32 control1 = { XM_PERMUTE_1X, XM_PERMUTE_0Z, XM_PERMUTE_0X, XM_PERMUTE_1Y };
		constexpr const XMVECTORI32 control2 = { XM_PERMUTE_0W, XM_PERMUTE_1X, XM_PERMUTE_0Z, XM_PERMUTE_0X };
		constexpr const XMVECTORI32 control3 = { XM_PERMUTE_1Y, XM_PERMUTE_0W, XM_PERMUTE_0X, XM_PERMUTE_0X };
		XMVECTOR vecA = XMVectorMultiply(XMVectorPermute(vecSC1, vecSC2, control1), Math::XNAExt::Swizzle(vecSC2, 2, 3, 3, 2));
		XMVECTOR vecB = XMVectorMultiply(XMVectorPermute(vecSC1, vecSC2, control2), Math::XNAExt::Swizzle(vecSC2, 3, 3, 2, 2));
		XMVECTOR vecC = XMVectorMultiply(XMVectorPermute(vecSC1, vecSC2, control3), Math::XNAExt::Swizzle(vecSC2, 3, 2, 1, 0));
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
XMMATRIX RenderObject::CreateWorldMatrix2D(Vector3f* const position, Vector2f* const angleX,
	Vector2f* const angleY, Vector2f* const angleZ, Vector3f* const scale, XMMATRIX* const camera)
{
	XMMATRIX mat = XMMatrixIdentity();
	if (scale->x != 1.0f || scale->y != 1.0f || scale->z != 1.0f) {
		mat = XMMatrixMultiply(mat, XMMatrixScaling(scale->x, scale->y, scale->z));
	}
	{
		XMVECTOR vecSC1 = XMVectorSet(angleX->x, angleX->y, angleX->y * angleY->y, angleX->y * angleY->x);
		XMVECTOR vecSC2 = XMVectorSet(angleY->x, angleY->y, angleZ->x, angleZ->y);

		constexpr const XMVECTORI32 control1 = { XM_PERMUTE_1X, XM_PERMUTE_0Z, XM_PERMUTE_0X, XM_PERMUTE_1Y };
		constexpr const XMVECTORI32 control2 = { XM_PERMUTE_0W, XM_PERMUTE_1X, XM_PERMUTE_0Z, XM_PERMUTE_0X };
		constexpr const XMVECTORI32 control3 = { XM_PERMUTE_1Y, XM_PERMUTE_0W, XM_PERMUTE_0X, XM_PERMUTE_0X };
		XMVECTOR vecA = XMVectorMultiply(XMVectorPermute(vecSC1, vecSC2, control1), Math::XNAExt::Swizzle(vecSC2, 2, 3, 3, 2));
		XMVECTOR vecB = XMVectorMultiply(XMVectorPermute(vecSC1, vecSC2, control2), Math::XNAExt::Swizzle(vecSC2, 3, 3, 2, 2));
		XMVECTOR vecC = XMVectorMultiply(XMVectorPermute(vecSC1, vecSC2, control3), Math::XNAExt::Swizzle(vecSC2, 3, 2, 1, 0));
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

void RenderObject::SetAngleX(float x) {
	if (angle_.x != x) {
		angle_.x = x;
		angleX_ = Vector2f(cosf(x), sinf(x));
	}
}
void RenderObject::SetAngleY(float y) {
	if (angle_.y != y) {
		angle_.y = y;
		angleY_ = Vector2f(cosf(y), sinf(y));
	}
}
void RenderObject::SetAngleZ(float z) {
	if (angle_.z != z) {
		angle_.z = z;
		angleZ_ = Vector2f(cosf(z), sinf(z));
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
	bufferVertex_ = std::shared_ptr<DataBuffer>(new DataBuffer(GL_ARRAY_BUFFER));
	bufferIndex_ = std::shared_ptr<DataBuffer>(new DataBuffer(GL_ELEMENT_ARRAY_BUFFER));
	bufferVertex_->Create(64U, sizeof(VertexTLX));
}
void StaticRenderObject::Update() {
}
void StaticRenderObject::Render() {
	if (shader_ == nullptr) return;

	WindowMain* window = WindowMain::GetBase();

	if (bufferVertex_->Bind()) {
		VertexTLX::BindAttribute();

		XMMATRIX matWorld = RenderObject::CreateWorldMatrix2D(&position_, &angleX_, &angleY_,
			&angleZ_, &scale_, window->GetProjectionMatrix());

		sf::Shader* pShader = shader_->GetData();
		sf::Texture::bind(texture_->GetData());
		sf::Shader::bind(pShader);

		//pShader->setUniform("g_mWorld", (Mat4&)matWorld);
		//pShader->setUniform("g_mViewProjection", (Mat4&)(*window->GetProjectionMatrix()));
		//pShader->setUniform("g_vScroll", sf::Glsl::Vec2(0, 0));
		//pShader->setUniform("g_vColor", (sf::Glsl::Vec4&)GLColor(color_));
		pShader->setUniform("g_vColor", (sf::Glsl::Vec4&)GLColor());

		if (index_.size() > 0) {
			bufferIndex_->Bind();
			glDrawElements((GLenum)primitiveType_, index_.size(), GL_UNSIGNED_SHORT, (LPVOID)0);
		}
		else {
			glDrawArrays((GLenum)primitiveType_, 0, vertex_.size());
		}

		sf::Texture::bind(nullptr);
		//sf::Shader::bind(nullptr);

		VertexTLX::UnbindAttribute();
		bufferVertex_->Unbind();
	}
}

void StaticRenderObject::UpdateVertexBuffer() {
	size_t sizeBuffer = bufferVertex_->GetSize();
	if (vertex_.size() > sizeBuffer) {
		while (vertex_.size() > sizeBuffer)
			sizeBuffer = sizeBuffer << 1;
		bufferVertex_->Create(sizeBuffer, sizeof(VertexTLX));
	}
	bufferVertex_->LoadData(vertex_.data(), vertex_.size(), GL_STATIC_DRAW);
}
void StaticRenderObject::UpdateIndexBuffer() {
	size_t sizeBuffer = bufferIndex_->GetSize();
	if (index_.size() > sizeBuffer) {
		while (index_.size() > sizeBuffer)
			sizeBuffer = sizeBuffer << 1;
		bufferIndex_->Create(sizeBuffer, sizeof(uint16_t));
	}
	bufferIndex_->LoadData(index_.data(), index_.size(), GL_STATIC_DRAW);
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
	primitiveType_ = PrimitiveType::Quads;

	bPermitCamera_ = true;

	Initialize();
}
Sprite2D::~Sprite2D() {
}

void Sprite2D::Initialize() {
	bufferVertex_ = std::shared_ptr<DataBuffer>(new DataBuffer(GL_ARRAY_BUFFER));
	bufferIndex_ = std::shared_ptr<DataBuffer>(new DataBuffer(GL_ELEMENT_ARRAY_BUFFER));
	bufferVertex_->Create(4U, sizeof(VertexTLX));
	vertex_.resize(4U, VertexTLX());
}

void Sprite2D::SetSourceRectNormalized(const GLRect& rc) {
	GetVertex(0)->texcoord = Vector2f(rc.left, rc.top);
	GetVertex(1)->texcoord = Vector2f(rc.right, rc.top);
	GetVertex(2)->texcoord = Vector2f(rc.left, rc.bottom);
	GetVertex(3)->texcoord = Vector2f(rc.right, rc.bottom);

	//UpdateVertexBuffer();
}
void Sprite2D::SetSourceRect(const GLRect& rc) {
	float width = texture_->GetData()->getSize().x;
	float height = texture_->GetData()->getSize().y;

	GetVertex(0)->texcoord = Vector2f(rc.left / width, rc.top / height);
	GetVertex(1)->texcoord = Vector2f(rc.right / width, rc.top / height);
	GetVertex(2)->texcoord = Vector2f(rc.left / width, rc.bottom / height);
	GetVertex(3)->texcoord = Vector2f(rc.right / width, rc.bottom / height);

	//UpdateVertexBuffer();
}
void Sprite2D::SetDestRect(const GLRect& rc) {
	GetVertex(0)->position = Vector3f(rc.left, rc.top, 1.0f);
	GetVertex(1)->position = Vector3f(rc.right, rc.top, 1.0f);
	GetVertex(2)->position = Vector3f(rc.left, rc.bottom, 1.0f);
	GetVertex(3)->position = Vector3f(rc.right, rc.bottom, 1.0f);

	//UpdateVertexBuffer();
}
void Sprite2D::SetDestCenter() {
	VertexTLX* vertTL = GetVertex(0);
	VertexTLX* vertBR = GetVertex(3);

	float width = texture_->GetData()->getSize().x;
	float height = texture_->GetData()->getSize().y;
	float vWidth = (vertBR->texcoord.x - vertTL->texcoord.x) * width / 2.0f;
	float vHeight = (vertBR->texcoord.y - vertTL->texcoord.y) * height / 2.0f;
	SetDestRect(GLRect(-vWidth, -vHeight, vWidth, vHeight));

	//UpdateVertexBuffer();
}