#pragma once

#include "../../pch.h"

#include "Utility.hpp"

class VertexTLX {
public:
	static const D3DVERTEXELEMENT9 VertexLayout[];
	static const D3DVERTEXELEMENT9 VertexLayoutInstanced[];
	static const size_t LayoutSize;
	static const DWORD VertexFormat;
public:
	D3DXVECTOR3 position;
	D3DCOLOR diffuse;
	D3DXVECTOR2 texcoord;
public:
	VertexTLX() : position(0, 0, 0), texcoord(0, 0), diffuse(0xffffffff) {}
	VertexTLX(const D3DXVECTOR3& pos) : 
		position(pos), texcoord(0, 0), diffuse(0xffffffff) {
	}
	VertexTLX(const D3DXVECTOR3& pos, const D3DXVECTOR2& tex) :
		position(pos), texcoord(tex), diffuse(0xffffffff) {
	}
	VertexTLX(const D3DXVECTOR3& pos, const D3DXVECTOR2& tex, const D3DCOLOR& col) :
		position(pos), texcoord(tex), diffuse(col) {
	}

	inline void Bias(float bias) {
		position.x += bias;
		position.y += bias;
	}
};
class InstanceData {
public:
	D3DCOLOR diffuse_color;
	D3DXVECTOR4 vec4a;
	D3DXVECTOR4 vec4b;
	D3DXVECTOR4 vec4c;
public:
	InstanceData() : diffuse_color(0xffffffff), vec4a(0, 0, 0, 0), vec4b(0, 0, 0, 0), vec4c(0, 0, 0, 0) {}
	InstanceData(D3DCOLOR col, CD3DXVECTOR4 va, CD3DXVECTOR4 vb, CD3DXVECTOR4 vc) :
		diffuse_color(col), vec4a(va), vec4b(vb), vec4c(vc) {}
};

struct BufferLockParameter {
	UINT lockOffset = 0U;
	DWORD lockFlag = 0U;
	void* data = nullptr;
	size_t dataCount = 0U;
	size_t dataStride = 1U;

	BufferLockParameter() {
		lockOffset = 0U;
		lockFlag = 0U;
		data = nullptr;
		dataCount = 0U;
		dataStride = 1U;
	};
	BufferLockParameter(DWORD _lockFlag) {
		lockOffset = 0U;
		lockFlag = _lockFlag;
		data = nullptr;
		dataCount = 0U;
		dataStride = 1U;
	};
	BufferLockParameter(LPVOID _data, size_t _count, size_t _stride, DWORD _lockFlag) {
		lockOffset = 0U;
		lockFlag = _lockFlag;
		data = 0U;
		dataCount = _count;
		dataStride = _stride;
	};

	template<typename T>
	void SetSource(T& vecSrc, size_t countMax, size_t _stride) {
		data = vecSrc.data();
		dataCount = std::min(countMax, vecSrc.size());
		dataStride = _stride;
	}
};

template<typename T>
class BufferBase {
	static_assert(std::is_base_of<IDirect3DResource9, T>::value, "T must be a Direct3D resource");
public:
	BufferBase(IDirect3DDevice9* device, DWORD usage);
	virtual ~BufferBase();

	inline void Release() { ptr_release(buffer_); }

	HRESULT UpdateBuffer(BufferLockParameter* pLock);

	virtual HRESULT Create(size_t size, size_t stride, D3DPOOL pool, DWORD* pParam) = 0;

	T* GetBuffer() { return buffer_; }
	size_t GetSize() { return size_; }
	size_t GetSizeInBytes() { return size_ * stride_; }
protected:
	IDirect3DDevice9* pDevice_;
	T* buffer_;
	size_t size_;
	size_t stride_;
	DWORD usage_;
};

class DxVertexBuffer : public BufferBase<IDirect3DVertexBuffer9> {
public:
	DxVertexBuffer(IDirect3DDevice9* device, DWORD usage);
	virtual ~DxVertexBuffer();

	//pParam: [FVF]
	virtual HRESULT Create(size_t size, size_t stride, D3DPOOL pool, DWORD* pParam);
private:
	DWORD fvf_;
};
class DxIndexBuffer : public BufferBase<IDirect3DIndexBuffer9> {
public:
	DxIndexBuffer(IDirect3DDevice9* device, DWORD usage);
	virtual ~DxIndexBuffer();

	//pParam: [D3DFORMAT]
	virtual HRESULT Create(size_t size, size_t stride, D3DPOOL pool, DWORD* pParam);
private:
	D3DFORMAT format_;
};

#define DX_MAX_BUFFER_SIZE 0x10000u
class VertexBufferManager : public DxResourceManagerBase {
	static VertexBufferManager* base_;
public:
	VertexBufferManager();
	virtual ~VertexBufferManager();

	static VertexBufferManager* const GetBase() { return base_; }

	void Initialize();
	void Release();

	void CreateBuffers();

	virtual void OnLostDevice();
	virtual void OnRestoreDevice();

	IDirect3DVertexDeclaration9* GetDeclaration(size_t index) { return listDeclaration_[index]; }
	IDirect3DVertexDeclaration9* GetDeclarationTLX() { return GetDeclaration(0); }
	IDirect3DVertexDeclaration9* GetDeclarationTLX_Instance() { return GetDeclaration(1); }

	DxVertexBuffer* GetDynamicVertexBuffer(size_t index) { return listBufferDynamicVertex_[index]; }
	DxVertexBuffer* GetDynamicVertexBufferTLX() { return GetDynamicVertexBuffer(0); }
	DxVertexBuffer* GetDynamicVertexBufferTLX_Instance() { return GetDynamicVertexBuffer(1); }
	DxIndexBuffer* GetDynamicIndexBuffer() { return bufferDynamicIndex_; }
private:
	std::vector<IDirect3DVertexDeclaration9*> listDeclaration_;

	std::vector<DxVertexBuffer*> listBufferDynamicVertex_;
	DxIndexBuffer* bufferDynamicIndex_;
};