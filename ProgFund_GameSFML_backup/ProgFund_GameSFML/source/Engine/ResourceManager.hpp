#pragma once
#include "../../pch.h"

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

class FontResource : public Resource {
public:
	FontResource();

	virtual void LoadFromFile(const std::string& path);
	virtual void UnloadResource();

	sf::Font* GetData() { return &font_; }
protected:
	sf::Font font_;
};
class TextureResource : public Resource {
public:
	TextureResource();

	virtual void LoadFromFile(const std::string& path) {
		LoadFromFile(path, false);
	}
	virtual void LoadFromFile(const std::string& path, bool bMipmap);
	virtual void UnloadResource();

	sf::Texture* GetData() { return &texture_; }
protected:
	sf::Texture texture_;
};
class SoundResource : public Resource {
public:
	SoundResource();

	virtual void LoadFromFile(const std::string& path);
	virtual void UnloadResource();

	sf::SoundBuffer* GetData() { return &buffer_; }
protected:
	sf::SoundBuffer buffer_;
};
class ShaderResource : public Resource {
public:
	enum class Type : uint8_t {
		Vertex = sf::Shader::Vertex,
		Fragment = sf::Shader::Fragment,
		Geometry = sf::Shader::Geometry,
		Fused = 0xff,
	};
public:
	ShaderResource();

	virtual void LoadFromFile(const std::string& path) {
		LoadFromFile(path, Type::Fragment);
	}
	virtual void LoadFromFile(const std::string& path, Type type);
	virtual void LoadFromFile(const std::string& pathVertex, const std::string& pathFragment);
	virtual void UnloadResource();

	sf::Shader* GetData() { return &shader_; }
	Type GetShaderType() { return typeShader_; }
protected:
	sf::Shader shader_;
	Type typeShader_;
};

class ResourceManager {
	static ResourceManager* base_;
public:
	ResourceManager();
	virtual ~ResourceManager();

	static ResourceManager* const GetBase() { return base_; }

	void Initialize();

	void AddResource(shared_ptr<Resource> resource, const std::string& path);
	void RemoveResource(const std::string& path);

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