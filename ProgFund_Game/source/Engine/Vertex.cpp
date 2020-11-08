#include "pch.h"

#include "DxConstant.hpp"
#include "Vertex.hpp"
#include "Window.hpp"

//*******************************************************************
//VertexTLX
//*******************************************************************
const D3DVERTEXELEMENT9 VertexTLX::VertexLayout[] = {
	{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
	{ 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END()
};
const D3DVERTEXELEMENT9 VertexTLX::VertexLayoutFlipped[] = {
	{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	{ 0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
	D3DDECL_END()
};
const size_t VertexTLX::LayoutSize = 3U;
const DWORD VertexTLX::VertexFormat = D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_DIFFUSE;

//*******************************************************************
//VertexBufferManager
//*******************************************************************
VertexBufferManager* VertexBufferManager::base_ = nullptr;
VertexBufferManager::VertexBufferManager() {
	bufferDynamicIndex_ = nullptr;
}
VertexBufferManager::~VertexBufferManager() {
}
void VertexBufferManager::Initialize() {
	if (base_) throw EngineError("VertexBufferManager already initialized.");
	base_ = this;

	WindowMain* window = WindowMain::GetBase();
	IDirect3DDevice9* device = window->GetDevice();

	window->AddDxResourceListener(this);

	//Load vertex declarations
	{
		HRESULT hr = S_OK;

		std::vector<std::pair<const char*, const D3DVERTEXELEMENT9*>> listDeclParam = {
			std::make_pair("TLX", VertexTLX::VertexLayout),
			std::make_pair("TLX_F", VertexTLX::VertexLayoutFlipped),
		};
		for (auto& itr : listDeclParam) {
			IDirect3DVertexDeclaration9* pDecl = nullptr;
			hr = device->CreateVertexDeclaration(itr.second, &pDecl);
			if (FAILED(hr)) {
				throw EngineError(StringUtility::Format("Failed to create vertex declaration for \"%s\".\n\t%s",
					itr.first, ErrorUtility::StringFromHResult(hr).c_str()));
			}
			listDeclaration_.push_back(pDecl);
		}
	}

	CreateBuffers();
}
void VertexBufferManager::Release() {
	WindowMain* window = WindowMain::GetBase();
	window->RemoveDxResourceListener(this);

	for (auto& iDecl : listDeclaration_)
		ptr_release(iDecl);

	for (auto& iBuffer : listBufferDynamicVertex_)
		ptr_delete(iBuffer);
	ptr_delete(bufferDynamicIndex_);
}

void VertexBufferManager::CreateBuffers() {
	WindowMain* window = WindowMain::GetBase();
	IDirect3DDevice9* device = window->GetDevice();

	HRESULT hr = S_OK;

	std::vector<std::pair<const char*, std::pair<size_t, DWORD>>> listVertexParam = {
		std::make_pair("TLX", std::make_pair(sizeof(VertexTLX), VertexTLX::VertexFormat)),
	};
	for (auto& itr : listVertexParam) {
		DxVertexBuffer* pBuffer = new DxVertexBuffer(device, D3DUSAGE_DYNAMIC);
		hr = pBuffer->Create(DX_MAX_BUFFER_SIZE, itr.second.first, D3DPOOL_DEFAULT, &itr.second.second);
		if (FAILED(hr)) {
			throw EngineError(StringUtility::Format("Failed to create vertex buffer for \"%s\".\n\t%s",
				itr.first, ErrorUtility::StringFromHResult(hr).c_str()));
		}
		listBufferDynamicVertex_.push_back(pBuffer);
	}

	{
		bufferDynamicIndex_ = new DxIndexBuffer(device, D3DUSAGE_DYNAMIC);
		DWORD fmt = D3DFMT_INDEX16;
		hr = bufferDynamicIndex_->Create(DX_MAX_BUFFER_SIZE, sizeof(uint16_t), D3DPOOL_DEFAULT, &fmt);
		if (FAILED(hr)) {
			throw EngineError(StringUtility::Format("Failed to create index buffer.\n\t%s",
				ErrorUtility::StringFromHResult(hr).c_str()));
		}
	}
}

void VertexBufferManager::OnLostDevice() {
	for (auto& iBuffer : listBufferDynamicVertex_)
		ptr_delete(iBuffer);
	ptr_delete(bufferDynamicIndex_);
}
void VertexBufferManager::OnRestoreDevice() {
	CreateBuffers();
}

//*******************************************************************
//BufferBase
//*******************************************************************
template class BufferBase<IDirect3DVertexBuffer9>;
template class BufferBase<IDirect3DIndexBuffer9>;

template<typename T>
BufferBase<T>::BufferBase(IDirect3DDevice9* device, DWORD usage) {
	pDevice_ = device;
	buffer_ = nullptr;
	size_ = 0U;
	stride_ = 0U;
	usage_ = usage;
}
template<typename T>
BufferBase<T>::~BufferBase() {
	Release();
}
template<typename T>
HRESULT BufferBase<T>::UpdateBuffer(BufferLockParameter* pLock) {
	HRESULT hr = S_OK;

	if (pLock == nullptr || buffer_ == nullptr) return E_POINTER;
	else if (pLock->lockOffset >= size_) return E_INVALIDARG;
	else if (pLock->dataCount == 0U || pLock->dataStride == 0U) return S_OK;

	size_t usableCount = size_ - pLock->lockOffset;
	size_t lockCopySize = std::min(pLock->dataCount, usableCount) * pLock->dataStride;

	void* tmp;
	hr = buffer_->Lock(pLock->lockOffset * pLock->dataStride, lockCopySize, &tmp, pLock->lockFlag);
	if (SUCCEEDED(hr)) {
		memcpy_s(tmp, size_ * stride_, pLock->data, lockCopySize);
		buffer_->Unlock();
	}

	return hr;
}

//*******************************************************************
//DxVertexBuffer
//*******************************************************************
DxVertexBuffer::DxVertexBuffer(IDirect3DDevice9* device, DWORD usage) : BufferBase(device, usage) {
	fvf_ = 0U;
}
DxVertexBuffer::~DxVertexBuffer() {
	Release();
}
HRESULT DxVertexBuffer::Create(size_t size, size_t stride, D3DPOOL pool, DWORD* pParam) {
	this->Release();
	size_ = size;
	stride_ = stride;
	fvf_ = pParam[0];
	return pDevice_->CreateVertexBuffer(size_ * stride_, usage_, fvf_,
		pool, &buffer_, nullptr);
}

//*******************************************************************
//DxIndexBuffer
//*******************************************************************
DxIndexBuffer::DxIndexBuffer(IDirect3DDevice9* device, DWORD usage) : BufferBase(device, usage) {
	pDevice_ = device;
	format_ = D3DFMT_INDEX16;
}
DxIndexBuffer::~DxIndexBuffer() {
}
HRESULT DxIndexBuffer::Create(size_t size, size_t stride, D3DPOOL pool, DWORD* pParam) {
	this->Release();
	size_ = size;
	stride_ = stride;
	format_ = (D3DFORMAT)pParam[0];
	return pDevice_->CreateIndexBuffer(size_ * stride_, usage_, format_, 
		pool, &buffer_, nullptr);
}