#include "pch.h"
#include "ResourceManager.hpp"
#include "Window.hpp"
#include "Vertex.hpp"
#include "Sound.hpp"

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

	WindowMain* window = WindowMain::GetBase();
	IDirect3DDevice9* device = window->GetDevice();

	window->AddDxResourceListener(this);

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
		shaderDefault_->LoadFromFile(PathProperty::GetWorkingDirectory() + "resource/shader/default_2d.fx");

		shaderDefault_->SetTechniqueByName("Render");

		this->AddResource(shaderDefault_, "__SHADER_DEFAULT__");
	}
}
void ResourceManager::Release() {
	WindowMain* window = WindowMain::GetBase();
	window->RemoveDxResourceListener(this);

	for (auto itr = mapResource_.begin(); itr != mapResource_.end(); ++itr) {
		itr->second->UnloadResource();
		itr->second = nullptr;
	}
}

void ResourceManager::OnLostDevice() {
	for (auto itr = mapResource_.begin(); itr != mapResource_.end(); ++itr) {
		itr->second->OnLostDevice();
	}
}
void ResourceManager::OnRestoreDevice() {
	for (auto itr = mapResource_.begin(); itr != mapResource_.end(); ++itr) {
		itr->second->OnRestoreDevice();
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
	type_ = Resource::Type::Texture;
	typeTexture_ = Type::Texture;
	ZeroMemory(&infoImage_, sizeof(D3DXIMAGE_INFO));
	texture_ = nullptr;
	surface_ = nullptr;
	pSurfaceSave_ = nullptr;
}
void TextureResource::LoadFromFile(const std::string& path, bool bMipmap) {
	IDirect3DDevice9* device = WindowMain::GetBase()->GetDevice();

	typeTexture_ = Type::Texture;
	path_ = path;

	//printf(StringFormat("Loaded texture resource [%s][%dx%d]\n", path.c_str(), 
	//	(int)texture_.getSize().x, (int)texture_.getSize().y).c_str());

	auto WrapError = [&](HRESULT hr) {
		if (FAILED(hr))
			throw EngineError(StringUtility::Format("Failed to load texture resource [%s]\n\t%s",
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

	typeTexture_ = Type::RenderTarget;
	path_ = name;

	auto WrapError = [&](HRESULT hr) {
		if (FAILED(hr))
			throw EngineError(StringUtility::Format("Failed to create render target [%s]\n\t%s",
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

void TextureResource::OnLostDevice() {
	if (typeTexture_ != Type::RenderTarget) return;

	IDirect3DDevice9* device = WindowMain::GetBase()->GetDevice();

	//Because IDirect3DDevice9::Reset requires me to delete all render targets, 
	//	this is used to copy back lost data when the render targets are recreated.
	HRESULT hr = device->CreateOffscreenPlainSurface(infoImage_.Width, infoImage_.Height, infoImage_.Format,
		D3DPOOL_SYSTEMMEM, &pSurfaceSave_, nullptr);
	if (SUCCEEDED(hr)) {
		hr = device->GetRenderTargetData(surface_, pSurfaceSave_);
		if (FAILED(hr)) {
			std::string err = StringUtility::Format("Texture::OnLostDevice: "
				"Failed to create temporary surface [%s]\n\t%s",
				path_.c_str(), ErrorUtility::StringFromHResult(hr).c_str());
			ptr_release(pSurfaceSave_);
			throw EngineError(err);
		}
	}

	ptr_release(texture_);
	ptr_release(surface_);
}
void TextureResource::OnRestoreDevice() {
	if (typeTexture_ != Type::RenderTarget) return;
	if (pSurfaceSave_ == nullptr) return;

	IDirect3DDevice9* device = WindowMain::GetBase()->GetDevice();

	CreateAsRenderTarget(path_, infoImage_.Width, infoImage_.Height);

	HRESULT hr = device->UpdateSurface(pSurfaceSave_, nullptr, surface_, nullptr);
	if (FAILED(hr)) {
		std::string err = StringUtility::Format("Texture::OnRestoreDevice: "
			"Failed to restore surface [%s]\n\t%s",
			path_.c_str(), ErrorUtility::StringFromHResult(hr).c_str());
		throw EngineError(err);
	}

	ptr_release(pSurfaceSave_);
}

//*******************************************************************
//SoundResource
//*******************************************************************
SoundResource::SoundResource() {
	type_ = Resource::Type::Sound;
	file_ = nullptr;
	buffer_ = nullptr;
	typeSound_ = Type::Unknown;
}
void SoundResource::LoadFromFile(const std::string& path) {
	path_ = path;

	auto SetError = [&](const std::string& error) {
		throw EngineError(StringUtility::Format("Failed to load sound resource [%s]\n\t%s",
			path.c_str(), error.c_str()));
	};

	GET_INSTANCE(SoundManager, soundManager);

	file_ = new std::ifstream();
	file_->open(path, std::ios::binary);
	if (!file_->is_open())
		SetError("Cannot open file for reading.");

	file_->seekg(0, std::ios::end);
	size_t fileSize = file_->tellg();
	file_->seekg(0, std::ios::beg);

	if (fileSize <= 0x100)
		SetError("Invalid file.");

	{
		char header[0x100];
		file_->read(header, 0x100);

		if (memcmp(header, "RIFF", 4) == 0) {
			typeSound_ = Type::Wave;

			size_t ptr = 0xC;
			while (ptr <= 0x100) {
				size_t chkSize = *(uint32_t*)&header[ptr + sizeof(uint32_t)];
				if (memcmp(&header[ptr], "fmt ", 4) == 0 && chkSize >= 0x10) {
					WAVEFORMATEX* wavefmt = (WAVEFORMATEX*)&header[ptr + sizeof(uint32_t) * 2];
					if (wavefmt->wFormatTag != WAVE_FORMAT_PCM)
						SetError("Unsupported wave format.");
					break;
				}
				ptr += chkSize + sizeof(uint32_t);
			}
		}
		else if (memcmp(header, "OggS", 4) == 0) {
			typeSound_ = Type::OggVorbis;
		}
	}

	switch (typeSound_) {
	case Type::Wave:
		buffer_ = new DxSoundSourceWave();
		break;
	case Type::OggVorbis:
		buffer_ = new DxSoundSourceStreamerOgg();
		break;
	}

	if (buffer_) {
		buffer_->SetManager(SoundManager::GetBase());
		buffer_->SetSource(file_);
		if (!buffer_->_CreateBuffer())
			goto lab_sound_fail;
		return;
	}
lab_sound_fail:
	SetError("Invalid or unsupported sound file; cannot create sound buffer.");
}
void SoundResource::UnloadResource() {
	ptr_delete(buffer_);
	if (file_) {
		file_->close();
		delete file_;
		file_ = nullptr;
	}
}
bool SoundResource::Play() {
	if (buffer_)
		return buffer_->Play();
	return false;
}
bool SoundResource::Pause() {
	if (buffer_)
		return buffer_->Pause();
	return false;
}
bool SoundResource::Stop() {
	if (buffer_)
		return buffer_->Stop();
	return false;
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
			std::string effectError = "NULL";
			if (pError)
				effectError = std::string((LPCSTR)pError->GetBufferPointer(), pError->GetBufferSize());
			throw EngineError(StringUtility::Format("Failed to load shader resource [%s][type=%s]\n\t%s\n%s",
				path.c_str(), strType, ErrorUtility::StringFromHResult(hr).c_str(), effectError.c_str()));
		}
	};

	DWORD flags = 0;
#ifdef _DEBUG
	flags |= D3DXSHADER_DEBUG;
#endif

	HRESULT hr = D3DXCreateEffectFromFileExA(device, path.c_str(), nullptr, nullptr, nullptr,
		flags, nullptr, &effect_, &pError);
	WrapError(hr);

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

void ShaderResource::OnLostDevice() {
	if (effect_ == nullptr) return;
	effect_->OnLostDevice();
}
void ShaderResource::OnRestoreDevice() {
	if (effect_ == nullptr) return;
	effect_->OnResetDevice();
}