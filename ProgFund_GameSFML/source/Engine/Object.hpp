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
public:
	ObjectBase();
	virtual ~ObjectBase();

	virtual void Initialize() {}
	virtual void Update() = 0;
	virtual void Render() = 0;

	void SetType(TypeObject type) { type_ = type; }
	TypeObject GetType() { return type_; }
	void SetRenderPriority(size_t pri) { renderPri_ = pri; }
	size_t GetRenderPriorityI() { return renderPri_; }
	bool IsVisible() { return bVisible_; }
	bool IsDeleted() { return bDeleted_; }

	bool IsObjectValueExists(const std::string& key) {
		return mapObjectValue_.find(key) != mapObjectValue_.end();
	}
	DWORD GetObjectValue(const std::string& key) { return mapObjectValue_[key]; }
	void SetObjectValue(const std::string& key, DWORD val) { mapObjectValue_[key] = val; }
	void DeleteObjectValue(const std::string& key) { mapObjectValue_.erase(key); }
private:
	TypeObject type_;
	size_t renderPri_;
	bool bVisible_;
	bool bDeleted_;

	std::unordered_map<std::string, DWORD> mapObjectValue_;
};

class RenderObject : public ObjectBase {
public:
	RenderObject();
	virtual ~RenderObject();

	virtual void Initialize() {}
	virtual void Update() = 0;
	virtual void Render() = 0;

	static XMMATRIX CreateWorldMatrix2D(D3DXVECTOR3* const position, D3DXVECTOR3* const angle,
		D3DXVECTOR3* const scale, XMMATRIX* const camera);
	static XMMATRIX CreateWorldMatrix2D(D3DXVECTOR3* const position, D3DXVECTOR2* const angleX,
		D3DXVECTOR2* const angleY, D3DXVECTOR2* const angleZ, D3DXVECTOR3* const scale, XMMATRIX* const camera);

	static size_t GetPrimitiveCount(D3DPRIMITIVETYPE type, size_t count);
	size_t GetPrimitiveCount();

	virtual void SetPosition(float x, float y, float z) { position_ = D3DXVECTOR3(x, y, z); }
	virtual void SetPosition(const D3DXVECTOR3& pos) { position_ = pos; }
	virtual void SetX(float x) { position_.x = x; }
	virtual void SetY(float y) { position_.y = y; }
	virtual void SetZ(float z) { position_.z = z; }

	virtual void SetAngle(float x, float y, float z) { 
		SetAngleX(x);
		SetAngleY(y);
		SetAngleZ(z);
	}
	virtual void SetAngle(const D3DXVECTOR3& pos) { 
		SetAngle(pos.x, pos.y, pos.z);
	}
	virtual void SetAngleX(float x);
	virtual void SetAngleY(float y);
	virtual void SetAngleZ(float z);

	virtual void SetScale(float x, float y, float z) { scale_ = D3DXVECTOR3(x, y, z); }
	virtual void SetScale(const D3DXVECTOR3& pos) { scale_ = pos; }
	virtual void SetScaleX(float x) { scale_.x = x; }
	virtual void SetScaleY(float y) { scale_.y = y; }
	virtual void SetScaleZ(float z) { scale_.z = z; }

	virtual void SetColor(DWORD rgb) { 
		SetColor((rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff);
	}
	virtual void SetColor(byte r, byte g, byte b) { 
		color_.x = r / 255.0f;
		color_.y = g / 255.0f;
		color_.z = b / 255.0f;
	}
	virtual void SetAlpha(byte alpha) { color_.w = alpha / 255.0f; }
	/*
	virtual void SetVertexColor(size_t index, DWORD rgb);
	virtual void SetVertexColor(size_t index, byte r, byte g, byte b);
	virtual void SetVertexAlpha(size_t index, byte alpha);
	*/
	virtual void SetVertex(size_t index, const VertexTLX& vertex);
	virtual VertexTLX* GetVertex(size_t index);

	void SetTexture(shared_ptr<TextureResource> texture) { 
		texture_ = texture ? texture : ResourceManager::GetBase()->GetEmptyTexture();
	}
	shared_ptr<TextureResource> GetTexture() { return texture_; }
	void SetShader(shared_ptr<ShaderResource> shader) { 
		shader_ = shader ? shader : ResourceManager::GetBase()->GetDefaultShader();
	}
	shared_ptr<ShaderResource> GetShader() { return shader_; }

	void SetPrimitiveType(D3DPRIMITIVETYPE type) { primitiveType_ = type; }
	D3DPRIMITIVETYPE GetPrimitiveType() { return primitiveType_; }
	void SetBlendType(BlendMode type) { blend_ = type; }
	BlendMode GetBlendType() { return blend_; }

	virtual void SetArrayVertex(const std::vector<VertexTLX>& vertices) { vertex_ = vertices; }
	virtual void SetArrayIndex(const std::vector<uint16_t>& indices) { index_ = indices; }
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
};

class StaticRenderObject : public RenderObject {
public:
	StaticRenderObject();
	virtual ~StaticRenderObject();

	virtual void Initialize();
	virtual void Update();
	virtual void Render();

	void UpdateVertexBuffer();
	void UpdateIndexBuffer();
	virtual void SetArrayVertex(const std::vector<VertexTLX>& vertices);
	virtual void SetArrayIndex(const std::vector<uint16_t>& indices);

	shared_ptr<DxVertexBuffer> GetVertexBuffer() { return bufferVertex_; }
	shared_ptr<DxIndexBuffer> GetIndexBuffer() { return bufferIndex_; }
protected:
	shared_ptr<DxVertexBuffer> bufferVertex_;
	shared_ptr<DxIndexBuffer> bufferIndex_;
};
//class DynamicRenderObject

class Sprite2D : public StaticRenderObject {
public:
	Sprite2D();
	~Sprite2D();

	virtual void Initialize();

	void SetSourceRectNormalized(const DxRect& rc);
	void SetSourceRect(const DxRect& rc);
	void SetDestRect(const DxRect& rc);
	void SetDestCenter();

	bool IsPermitCamera() { return bPermitCamera_; }
	void SetPermitCamera(bool bPermit) { bPermitCamera_ = bPermit; }
private:
	bool bPermitCamera_;
};