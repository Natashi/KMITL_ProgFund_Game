#include "pch.h"
#include "Utility.hpp"
#include "ResourceManager.hpp"

ResourceManager* ResourceManager::base_ = nullptr;
ResourceManager::ResourceManager() {
}
ResourceManager::~ResourceManager() {
}
void ResourceManager::Initialize() {
	if (base_) throw EngineError("ResourceManager already initialized.");
	base_ = this;
}
void ResourceManager::AddResource(shared_ptr<Resource> resource, const std::string& path) {
	resource->manager_ = this;
	auto itrFind = mapResource_.find(path);
	if (itrFind != mapResource_.end())
		itrFind->second = resource;
	else
		mapResource_.insert(std::make_pair(path, resource));
}
void ResourceManager::RemoveResource(const std::string& path) {
	auto itrFind = mapResource_.find(path);
	if (itrFind != mapResource_.end())
		mapResource_.erase(itrFind);
}
shared_ptr<Resource> ResourceManager::GetResource(const std::string& path) {
	auto itrFind = mapResource_.find(path);
	if (itrFind == mapResource_.end()) return nullptr;
	return itrFind->second;
}

Resource::Resource() {
	manager_ = nullptr;
	type_ = Type::Null;
	path_ = "";
}
Resource::~Resource() {
}

FontResource::FontResource() {
}
void FontResource::LoadFromFile(const std::string& path) {
	path_ = path;

	bool res = font_.loadFromFile(path);
	if (!res)
		throw EngineError(StringFormat("Failed to load font resource [%s]\n", path.c_str()));

	printf(StringFormat("Loaded font resource [%s][%s]\n", path.c_str(), font_.getInfo().family.c_str()).c_str());
}
void FontResource::UnloadResource() {
	font_.~Font();
}

TextureResource::TextureResource() {
}
void TextureResource::LoadFromFile(const std::string& path, bool bMipmap) {
	path_ = path;

	bool res = texture_.loadFromFile(path);
	if (!res)
		throw EngineError(StringFormat("Failed to load texture resource [%s]\n", path.c_str()));
	if (bMipmap) texture_.generateMipmap();

	printf(StringFormat("Loaded texture resource [%s][%dx%d]\n", path.c_str(), 
		(int)texture_.getSize().x, (int)texture_.getSize().y).c_str());
}
void TextureResource::UnloadResource() {
	texture_.~Texture();
}

ShaderResource::ShaderResource() {
}
void ShaderResource::LoadFromFile(const std::string& path, sf::Shader::Type type) {
	path_ = path;
	typeShader_ = type;

	const char* strType = "Invalid";
	switch (type) {
	case sf::Shader::Vertex:
		strType = "Vertex"; break;
	case sf::Shader::Geometry:
		strType = "Geometry"; break;
	case sf::Shader::Fragment:
		strType = "Fragment"; break;
	}

	bool res = shader_.loadFromFile(path, type);
	if (!res)
		throw EngineError(StringFormat("Failed to load shader resource [%s][type=%s]\n", path.c_str(), strType));

	printf(StringFormat("Loaded shader resource [%s][type=%s]\n", path.c_str(), strType).c_str());
}
void ShaderResource::UnloadResource() {
	shader_.~Shader();
}