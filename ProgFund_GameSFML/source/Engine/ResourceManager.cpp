#include "pch.h"
#include "ResourceManager.hpp"
#include "Window.hpp"
#include "Vertex.hpp"

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

	IDirect3DDevice9* device = WindowMain::GetBase()->GetDevice();

	{
		HRESULT hr = S_OK;

		textureEmpty_ = std::make_shared<TextureResource>();
		textureEmpty_->CreateAsRenderTarget("__TEXTURE_NULL__", 16, 16);
		{
			IDirect3DTexture9* texture = textureEmpty_->GetTexture();

			IDirect3DSurface9* surfaceTmp = nullptr;
			hr = device->CreateOffscreenPlainSurface(16, 16, textureEmpty_->GetImageInfo()->Format,
				D3DPOOL_SYSTEMMEM, &surfaceTmp, nullptr);
			if (FAILED(hr)) {
				throw EngineError(StringUtility::Format("TextureManager: Failed to create temporary surface.\n\t%s",
					ErrorUtility::StringFromHResult(hr).c_str()));
			}
			hr = device->GetRenderTargetData(textureEmpty_->GetSurface(), surfaceTmp);
			if (FAILED(hr)) {
				throw EngineError(StringUtility::Format("TextureManager: Failed to copy surface.\n\t%s",
					ErrorUtility::StringFromHResult(hr).c_str()));
			}
			{
				D3DLOCKED_RECT lockRect;
				surfaceTmp->LockRect(&lockRect, nullptr, D3DLOCK_DISCARD);
				memset(lockRect.pBits, 0xff, lockRect.Pitch * 16U);
				surfaceTmp->UnlockRect();
			}
			hr = device->UpdateSurface(surfaceTmp, nullptr, textureEmpty_->GetSurface(), nullptr);
			if (FAILED(hr)) {
				throw EngineError(StringUtility::Format("TextureManager: Failed to update surface.\n\t%s",
					ErrorUtility::StringFromHResult(hr).c_str()));
			}
			surfaceTmp->Release();
		}
		this->AddResource(textureEmpty_, "__TEXTURE_NULL__");
	}
	{
		shaderDefault_ = std::make_shared<ShaderResource>();
		shaderDefault_->LoadFromFile(PathProperty::GetModuleDirectory() + "resource/shader/default_2d.fx");

		shaderDefault_->SetTechniqueByName("Render");

		this->AddResource(shaderDefault_, "__SHADER_DEFAULT__");
	}
}
void ResourceManager::Release() {
	for (auto itr = mapResource_.begin(); itr != mapResource_.end(); ++itr) {
		itr->second->UnloadResource();
		itr->second = nullptr;
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

/*
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
*/

//*******************************************************************
//TextureResource
//*******************************************************************
TextureResource::TextureResource() {
	type_ = Type::Texture;
	ZeroMemory(&infoImage_, sizeof(D3DXIMAGE_INFO));
	texture_ = nullptr;
	surface_ = nullptr;
}
void TextureResource::LoadFromFile(const std::string& path, bool bMipmap) {
	IDirect3DDevice9* device = WindowMain::GetBase()->GetDevice();

	path_ = path;

	//printf(StringFormat("Loaded texture resource [%s][%dx%d]\n", path.c_str(), 
	//	(int)texture_.getSize().x, (int)texture_.getSize().y).c_str());

	auto WrapError = [&](HRESULT hr) {
		if (FAILED(hr))
			throw EngineError(StringUtility::Format("Failed to load texture resource [%s]\n\t[%s]",
				path.c_str(), ErrorUtility::StringFromHResult(hr).c_str()));
	};

	WrapError(D3DXGetImageInfoFromFileA(path.c_str(), &infoImage_));
	WrapError(D3DXCreateTextureFromFileExA(device, path.c_str(), 
		D3DX_DEFAULT, D3DX_DEFAULT,
		bMipmap ? infoImage_.MipLevels : 0, 0, infoImage_.Format, D3DPOOL_DEFAULT, D3DX_FILTER_BOX,
		D3DX_DEFAULT, 0x00000000, nullptr, nullptr, &texture_));

	if (bMipmap)
		texture_->GenerateMipSubLevels();

	WrapError(texture_->GetSurfaceLevel(0, &surface_));
}
void TextureResource::CreateAsRenderTarget(const std::string& name, size_t width, size_t height) {
	IDirect3DDevice9* device = WindowMain::GetBase()->GetDevice();

	path_ = name;

	auto WrapError = [&](HRESULT hr) {
		if (FAILED(hr))
			throw EngineError(StringUtility::Format("Failed to create render target [%s]\n\t[%s]",
				name.c_str(), ErrorUtility::StringFromHResult(hr).c_str()));
	};

	if (FAILED(device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8B8G8R8,
		D3DPOOL_DEFAULT, &texture_, nullptr)))
	{
		if (width > height) height = width;
		else if (height > width) width = height;

		WrapError(device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8B8G8R8,
			D3DPOOL_DEFAULT, &texture_, nullptr));
	}
	texture_->GenerateMipSubLevels();
	WrapError(texture_->GetSurfaceLevel(0, &surface_));

	infoImage_.Width = width;
	infoImage_.Height = height;
	infoImage_.Depth = 1;
	infoImage_.MipLevels = 1;
	infoImage_.Format = D3DFMT_A8B8G8R8;
	infoImage_.ResourceType = D3DRTYPE_TEXTURE;
	infoImage_.ImageFileFormat = D3DXIFF_BMP;
}
void TextureResource::UnloadResource() {
	ptr_release(surface_);
	ptr_release(texture_);
}

/*
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
*/

//*******************************************************************
//ShaderResource
//*******************************************************************
ShaderResource::ShaderResource() {
	type_ = Resource::Type::Shader;
	ZeroMemory(&effectDesc_, sizeof(D3DXEFFECT_DESC));
	effect_ = nullptr;
	technique_ = "";
	typeShader_ = Type::Unspecified;
}
void ShaderResource::LoadFromFile(const std::string& path, Type type) {
	IDirect3DDevice9* device = WindowMain::GetBase()->GetDevice();

	path_ = path;
	typeShader_ = type;

	const char* strType = "Unspecified";
	switch (type) {
	case Type::Vertex:
		strType = "Vertex"; break;
	case Type::Pixel:
		strType = "Pixel"; break;
	case Type::Geometry:
		strType = "Geometry"; break;
	case Type::Hull:
		strType = "Hull"; break;
	}

	ID3DXBuffer* pError = nullptr;

	auto WrapError = [&](HRESULT hr) {
		if (FAILED(hr)) {
			std::string effectError = std::string((LPCSTR)pError->GetBufferPointer(), pError->GetBufferSize());
			throw EngineError(StringUtility::Format("Failed to load shader resource [%s][type=%s]\n\t[%s]\n%s",
				path.c_str(), strType, ErrorUtility::StringFromHResult(hr).c_str(), effectError.c_str()));
		}
	};

	WrapError(D3DXCreateEffectFromFileExA(device, path.c_str(), nullptr, nullptr, nullptr,
		0, nullptr, &effect_, &pError));

	WrapError(effect_->GetDesc(&effectDesc_));
	if (effectDesc_.Techniques > 0)
		this->SetTechnique(effect_->GetTechnique(0));

	//printf(StringFormat("Loaded shader resource [%s][type=%s]\n", path.c_str(), strType).c_str());
}
void ShaderResource::UnloadResource() {
	ptr_release(effect_);
}
D3DXHANDLE ShaderResource::SetTechniqueByName(const char* name) {
	if (effect_ == nullptr) return "";
	D3DXHANDLE handle = effect_->GetTechniqueByName(name);
	SetTechnique(handle);
	return handle;
}
void ShaderResource::SetTechnique(D3DXHANDLE name) {
	if (effect_ == nullptr) return;
	effect_->SetTechnique(name);
}