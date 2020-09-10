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

	virtual void LoadFromFile(const std::string& path, bool bMipmap = false);
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
	ShaderResource();

	virtual void LoadFromFile(const std::string& path, sf::Shader::Type type);
	virtual void UnloadResource();

	sf::Shader* GetData() { return &shader_; }
	sf::Shader::Type GetShaderType() { return typeShader_; }
protected:
	sf::Shader shader_;
	sf::Shader::Type typeShader_;
};

class ResourceManager {
	static ResourceManager* base_;
public:
	ResourceManager();
	~ResourceManager();

	static ResourceManager* const GetBase() { return base_; }

	void Initialize();

	void AddResource(shared_ptr<Resource> resource, const std::string& path);
	void RemoveResource(const std::string& path);

	shared_ptr<Resource> GetResource(const std::string& path);
private:
	std::map<std::string, shared_ptr<Resource>> mapResource_;
};