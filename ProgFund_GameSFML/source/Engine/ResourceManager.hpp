#pragma once
#include "../../pch.h"

#include "Utility.hpp"

class ResourceManager;
class Resource {
	friend class ResourceManager;
public:
	enum class Type : byte {
		Null,
		Font,
		Texture,
		Sound,
		Shader,
	};
public:
	Resource();
	virtual ~Resource();

	virtual void LoadFromFile(const std::string& path) = 0;
	virtual void UnloadResource() = 0;

	ResourceManager* GetManager() { return manager_; }

	void SetType(Type t) { type_ = t; }
	Type GetType() { return type_; }

	void SetPath(const std::string& path) { path_ = path; }
	std::string GetPath() { return path_; }
protected:
	ResourceManager* manager_;
	Type type_;
	std::string path_;
};
/*
class FontResource : public Resource {
public:
	FontResource();

	virtual void LoadFromFile(const std::string& path);
	virtual void UnloadResource();

	sf::Font* GetData() { return &font_; }
protected:
	sf::Font font_;
};
*/
class TextureResource : public Resource {
public:
	TextureResource();

	virtual void LoadFromFile(const std::string& path) {
		LoadFromFile(path, false);
	}
	virtual void LoadFromFile(const std::string& path, bool bMipmap);
	virtual void CreateAsRenderTarget(const std::string& name, size_t width, size_t height);
	virtual void UnloadResource();

	D3DXIMAGE_INFO* GetImageInfo() { return &infoImage_; }

	IDirect3DTexture9* GetTexture() { return texture_; }
	IDirect3DSurface9* GetSurface() { return surface_; }
protected:
	D3DXIMAGE_INFO infoImage_;
	IDirect3DTexture9* texture_;
	IDirect3DSurface9* surface_;
};
/*
class SoundResource : public Resource {
public:
	SoundResource();

	virtual void LoadFromFile(const std::string& path);
	virtual void UnloadResource();

	sf::SoundBuffer* GetData() { return &buffer_; }
protected:
	sf::SoundBuffer buffer_;
};
*/
class ShaderResource : public Resource {
public:
	enum class Type : uint8_t {
		Vertex,
		Pixel,
		Geometry,
		Hull,
		Unspecified,
	};
public:
	ShaderResource();

	virtual void LoadFromFile(const std::string& path) {
		LoadFromFile(path, Type::Unspecified);
	}
	virtual void LoadFromFile(const std::string& path, Type type);
	virtual void UnloadResource();

	D3DXEFFECT_DESC* GetEffectDesc() { return &effectDesc_; }
	ID3DXEffect* GetEffect() { return effect_; }
	Type GetShaderType() { return typeShader_; }

	D3DXHANDLE GetTechnique() { return technique_; }
	D3DXHANDLE SetTechniqueByName(const char* name);
	void SetTechnique(D3DXHANDLE name);
protected:
	D3DXEFFECT_DESC effectDesc_;
	ID3DXEffect* effect_;
	D3DXHANDLE technique_;
	Type typeShader_;
};

class ResourceManager {
	static ResourceManager* base_;
public:
	ResourceManager();
	virtual ~ResourceManager();

	static ResourceManager* const GetBase() { return base_; }

	void Initialize();
	void Release();

	void AddResource(shared_ptr<Resource> resource, const std::string& path);
	void RemoveResource(const std::string& path);
	template<typename T> inline shared_ptr<T> LoadResource(const std::string& path, const std::string& name) {
		shared_ptr<T> res = std::make_shared<T>();
		res->LoadFromFile(PathProperty::GetModuleDirectory() + path);
		this->AddResource(res, name);
		return res;
	}

	shared_ptr<Resource> GetResource(const std::string& path);
	template<class T> shared_ptr<T> GetResourceAs(const std::string& path) {
		return std::dynamic_pointer_cast<T>(GetResource(path));
	}

	shared_ptr<TextureResource> GetEmptyTexture() { return textureEmpty_; }
	shared_ptr<ShaderResource> GetDefaultShader() { return shaderDefault_; }
private:
	std::map<std::string, shared_ptr<Resource>> mapResource_;

	shared_ptr<TextureResource> textureEmpty_;
	shared_ptr<ShaderResource> shaderDefault_;
};