#pragma once

#include "../../pch.h"

#include "Vertex.hpp"
#include "../Engine/ResourceManager.hpp"
#include "../Engine/Window.hpp"

enum class TypeObject : uint8_t {
	Null,
	Render,
	Sound,
	Player,
	Enemy,
	Shot,
	Item,
};

class ObjectBase {
private:
	TypeObject type_;
	size_t renderPri_;
	bool bVisible_;
	bool bDeleted_;

	std::unordered_map<std::string, DWORD> mapObjectValue_;
public:
	ObjectBase();
	virtual ~ObjectBase();

	virtual void Initialize() {}
	virtual void Update() = 0;
	virtual HRESULT Render() = 0;

	void SetType(TypeObject type) { type_ = type; }
	TypeObject GetType() { return type_; }
	void SetRenderPriority(size_t pri) { renderPri_ = pri; }
	size_t GetRenderPriorityI() { return renderPri_; }
	bool IsVisible() { return bVisible_; }
	void SetDeleted(bool b) { bDeleted_ = b; }
	bool IsDeleted() { return bDeleted_; }

	bool IsObjectValueExists(const std::string& key) {
		return mapObjectValue_.find(key) != mapObjectValue_.end();
	}
	DWORD GetObjectValue(const std::string& key) { return mapObjectValue_[key]; }
	void SetObjectValue(const std::string& key, DWORD val) { mapObjectValue_[key] = val; }
	void DeleteObjectValue(const std::string& key) { mapObjectValue_.erase(key); }
};

class RenderObject : public ObjectBase {
protected:
	D3DXVECTOR3 position_;
	D3DXVECTOR3 angle_;
	D3DXVECTOR2 angleX_;	//[cos, sin]
	D3DXVECTOR2 angleY_;	//[cos, sin]
	D3DXVECTOR2 angleZ_;	//[cos, sin]
	D3DXVECTOR3 scale_;
	D3DXVECTOR4 color_;

	shared_ptr<TextureResource> texture_;
	shared_ptr<ShaderResource> shader_;

	D3DPRIMITIVETYPE primitiveType_;
	BlendMode blend_;

	std::vector<VertexTLX> vertex_;
	std::vector<uint16_t> index_;
public:
	RenderObject();
	virtual ~RenderObject();

	virtual void Initialize() {}
	virtual void Update() = 0;
	virtual HRESULT Render() = 0;

	static D3DXMATRIX CreateWorldMatrix2D(D3DXVECTOR3* const position, D3DXVECTOR3* const angle,
		D3DXVECTOR3* const scale, D3DXMATRIX* const camera);
	static D3DXMATRIX CreateWorldMatrix2D(D3DXVECTOR3* const position, D3DXVECTOR2* const angleX,
		D3DXVECTOR2* const angleY, D3DXVECTOR2* const angleZ, D3DXVECTOR3* const scale, D3DXMATRIX* const camera);

	static size_t GetPrimitiveCount(D3DPRIMITIVETYPE type, size_t count);
	size_t GetPrimitiveCount();

	virtual void SetPosition(float x, float y, float z) { position_ = D3DXVECTOR3(x, y, z); }
	virtual void SetPosition(CD3DXVECTOR2 pos) { position_.x = pos.x; position_.y = pos.y; }
	virtual void SetPosition(CD3DXVECTOR3 pos) { position_ = pos; }
	virtual void SetX(float x) { position_.x = x; }
	virtual void SetY(float y) { position_.y = y; }
	virtual void SetZ(float z) { position_.z = z; }
	const D3DXVECTOR3& GetPosition() const { return position_; }

	virtual void SetAngle(float x, float y, float z) { 
		SetAngleX(x);
		SetAngleY(y);
		SetAngleZ(z);
	}
	virtual void SetAngle(CD3DXVECTOR3 angle) {
		SetAngle(angle.x, angle.y, angle.z);
	}
	virtual void SetAngleX(float x);
	virtual void SetAngleY(float y);
	virtual void SetAngleZ(float z);
	const D3DXVECTOR3& GetAngle() const { return angle_; }

	virtual void SetScale(float x, float y, float z) { scale_ = D3DXVECTOR3(x, y, z); }
	virtual void SetScale(CD3DXVECTOR3 scale) { scale_ = scale; }
	virtual void SetScaleX(float x) { scale_.x = x; }
	virtual void SetScaleY(float y) { scale_.y = y; }
	virtual void SetScaleZ(float z) { scale_.z = z; }
	const D3DXVECTOR3& GetScale() const { return scale_; }

	virtual void SetColor(DWORD rgb) { 
		SetColor((rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff);
	}
	virtual void SetColor(int r, int g, int b) {
		SetColor(D3DXVECTOR3(ColorUtility::ClampRet(r), ColorUtility::ClampRet(g), ColorUtility::ClampRet(b)) / 255.0f);
	}
	virtual void SetColor(const D3DXVECTOR3& col) {
		color_.x = col.x;
		color_.y = col.y;
		color_.z = col.z;
	}
	virtual void SetAlpha(int alpha) { color_.w = ColorUtility::ClampRet(alpha) / 255.0f; }
	/*
	virtual void SetVertexColor(size_t index, DWORD rgb);
	virtual void SetVertexColor(size_t index, byte r, byte g, byte b);
	virtual void SetVertexAlpha(size_t index, byte alpha);
	*/
	virtual void SetVertex(size_t index, const VertexTLX& vertex);
	virtual VertexTLX* GetVertex(size_t index);

	virtual void SetTexture(shared_ptr<TextureResource> texture) { 
		//texture_ = texture ? texture : ResourceManager::GetBase()->GetEmptyTexture();
		texture_ = texture;
	}
	shared_ptr<TextureResource> GetTexture() { return texture_; }
	virtual void SetShader(shared_ptr<ShaderResource> shader) {
		shader_ = shader ? shader : ResourceManager::GetBase()->GetDefaultShader();
	}
	shared_ptr<ShaderResource> GetShader() { return shader_; }

	void SetPrimitiveType(D3DPRIMITIVETYPE type) { primitiveType_ = type; }
	D3DPRIMITIVETYPE GetPrimitiveType() { return primitiveType_; }
	void SetBlendType(BlendMode type) { blend_ = type; }
	BlendMode GetBlendType() { return blend_; }

	virtual void SetArrayVertex(const std::vector<VertexTLX>& vertices) {
		vertex_ = vertices.size() <= DX_MAX_BUFFER_SIZE ? (vertices) :
			(std::vector<VertexTLX>(vertices.begin(), vertices.begin() + DX_MAX_BUFFER_SIZE));
	}
	virtual void SetArrayIndex(const std::vector<uint16_t>& indices) {
		index_ = indices.size() <= DX_MAX_BUFFER_SIZE ? (indices) :
			(std::vector<uint16_t>(indices.begin(), indices.begin() + DX_MAX_BUFFER_SIZE));
	}
};

class DynamicRenderObject : public RenderObject {
protected:
	virtual HRESULT _Render(DxVertexBuffer* bufferVertex, DxIndexBuffer* bufferIndex);
public:
	DynamicRenderObject();
	virtual ~DynamicRenderObject();

	virtual void Initialize();
	virtual void Update();
	virtual HRESULT Render() = 0;
};
class StaticRenderObject : public DynamicRenderObject {
protected:
	shared_ptr<DxVertexBuffer> bufferVertex_;
	shared_ptr<DxIndexBuffer> bufferIndex_;
	D3DXVECTOR2 scroll_;
public:
	StaticRenderObject();
	virtual ~StaticRenderObject();

	virtual void Initialize();
	virtual void Update();
	virtual HRESULT Render() = 0;

	void UpdateVertexBuffer();
	void UpdateIndexBuffer();
	virtual void SetArrayVertex(const std::vector<VertexTLX>& vertices) {
		RenderObject::SetArrayVertex(vertices);
		UpdateVertexBuffer();
	}
	virtual void SetArrayIndex(const std::vector<uint16_t>& indices) {
		RenderObject::SetArrayIndex(indices);
		UpdateIndexBuffer();
	}

	shared_ptr<DxVertexBuffer> GetVertexBuffer() { return bufferVertex_; }
	shared_ptr<DxIndexBuffer> GetIndexBuffer() { return bufferIndex_; }

	void SetScroll(float x, float y) { SetScroll(D3DXVECTOR2(x, y)); }
	void SetScroll(CD3DXVECTOR2 scroll) { scroll_ = scroll; }
	void SetScrollX(float x) { scroll_.x = x; }
	void SetScrollY(float y) { scroll_.y = y; }
};

class StaticRenderObject2D : public StaticRenderObject {
protected:
	bool bPermitCamera_;
public:
	StaticRenderObject2D();
	virtual ~StaticRenderObject2D();

	virtual HRESULT Render();

	bool IsPermitCamera() { return bPermitCamera_; }
	void SetPermitCamera(bool bPermit) { bPermitCamera_ = bPermit; }
};
class DynamicRenderObject2D : public DynamicRenderObject {
protected:
	bool bPermitCamera_;
public:
	DynamicRenderObject2D();
	virtual ~DynamicRenderObject2D();

	virtual HRESULT Render();

	bool IsPermitCamera() { return bPermitCamera_; }
	void SetPermitCamera(bool bPermit) { bPermitCamera_ = bPermit; }
};

class Sprite2D : public StaticRenderObject2D {
public:
	Sprite2D();
	~Sprite2D();

	virtual void Initialize();

	void SetSourceRectNormalized(const DxRectangle<float>& rc);
	void SetSourceRect(const DxRectangle<int>& rc);
	void SetDestRect(const DxRectangle<float>& rc);
	void SetDestCenter();
};