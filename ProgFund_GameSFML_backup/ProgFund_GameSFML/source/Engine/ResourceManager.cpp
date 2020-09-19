#include "pch.h"
#include "Utility.hpp"
#include "ResourceManager.hpp"

//*******************************************************************
//ResourceManager
//*******************************************************************
ResourceManager* ResourceManager::base_ = nullptr;
ResourceManager::ResourceManager() {
}
ResourceManager::~ResourceManager() {
}
void ResourceManager::Initialize() {
	if (base_) throw EngineError("ResourceManager already initialized.");
	base_ = this;

	{
		textureEmpty_ = std::make_shared<TextureResource>();
		{
			sf::Texture* pTexture = textureEmpty_->GetData();
			if (!pTexture->create(4, 4))
				throw EngineError("TextureManager: Failed to generate an empty texture.");

			byte* colorData = new byte[4 * 4 * 4];	//32-bit RGBA
			memset(colorData, 0x00, 4 * 4 * 4);
			pTexture->update((sf::Uint8*)colorData, 4, 4, 0, 0);
			delete[] colorData;
		}
		this->AddResource(textureEmpty_, "__TEXTURE_NULL__");
	}
	{
		shaderDefault_ = std::make_shared<ShaderResource>();
		shaderDefault_->LoadFromFile(PathProperty::GetModuleDirectory() + "resource/shader/default_vertex.glsl",
			PathProperty::GetModuleDirectory() + "resource/shader/default_fragment.glsl");
		this->AddResource(shaderDefault_, "__SHADER_DEFAULT__");
	}
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

//*******************************************************************
//Resource
//*******************************************************************
Resource::Resource() {
	manager_ = nullptr;
	type_ = Type::Null;
	path_ = "";
}
Resource::~Resource() {
}

//*******************************************************************
//FontResource
//*******************************************************************
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

//*******************************************************************
//TextureResource
//*******************************************************************
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

//*******************************************************************
//SoundResource
//*******************************************************************
SoundResource::SoundResource() {
}
void SoundResource::LoadFromFile(const std::string& path) {
	path_ = path;

	bool res = buffer_.loadFromFile(path);
	if (!res)
		throw EngineError(StringFormat("Failed to load sound resource [%s]\n", path.c_str()));

	printf(StringFormat("Loaded sound resource [%s][%uCh %uHz]\n", path.c_str(),
		buffer_.getChannelCount(), buffer_.getSampleRate()).c_str());
}
void SoundResource::UnloadResource() {
	buffer_.~SoundBuffer();
}

//*******************************************************************
//ShaderResource
//*******************************************************************
ShaderResource::ShaderResource() {
}
void ShaderResource::LoadFromFile(const std::string& path, Type type) {
	path_ = path;
	typeShader_ = type;

	const char* strType = "Invalid";
	switch (type) {
	case Type::Vertex:
		strType = "Vertex"; break;
	case Type::Geometry:
		strType = "Geometry"; break;
	case Type::Fragment:
		strType = "Fragment"; break;
	}

	bool res = shader_.loadFromFile(path, (sf::Shader::Type)type);
	if (!res)
		throw EngineError(StringFormat("Failed to load shader resource [%s][type=%s]\n", path.c_str(), strType));

	printf(StringFormat("Loaded shader resource [%s][type=%s]\n", path.c_str(), strType).c_str());
}
void ShaderResource::LoadFromFile(const std::string& pathVertex, const std::string& pathFragment) {
	path_ = pathVertex + " + " + pathFragment;
	typeShader_ = Type::Fused;

	bool res = shader_.loadFromFile(pathVertex, pathFragment);
	if (!res)
		throw EngineError(StringFormat("Failed to load shader resource [%s,%s][type=Fused]\n", 
			pathVertex.c_str(), pathFragment.c_str()));

	printf(StringFormat("Loaded shader resource [%s,%s][type=Fused]\n", 
		pathVertex.c_str(), pathFragment.c_str()).c_str());
}
void ShaderResource::UnloadResource() {
	shader_.~Shader();
}