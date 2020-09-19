#pragma once

#include "../../pch.h"

#include "Utility.hpp"

enum class PrimitiveType : uint16_t {
	PointList = GL_POINTS,
	LineList = GL_LINES,
	LineLoop = GL_LINE_LOOP,
	LineStrip = GL_LINE_STRIP,
	TriangleList = GL_TRIANGLES,
	TriangleStrip = GL_TRIANGLE_STRIP,
	TriangleFan = GL_TRIANGLE_FAN,
	Quads = GL_QUADS,
	QuadStrip = GL_QUAD_STRIP,
	Polygon = GL_POLYGON,
};

class VertexTLX {
public:
	Vector3f position;
	Vector2f texcoord;
	DWORD diffuse;

	VertexTLX() : position(0, 0, 0), texcoord(0, 0), diffuse(0xffffffff) {}
	VertexTLX(const Vector3f& pos, const Vector2f& tex) :
		position(pos), texcoord(tex), diffuse(0xffffffff) {}
	VertexTLX(const Vector3f& pos, const Vector2f& tex, const DWORD& col) :
		position(pos), texcoord(tex), diffuse(col) {}

	static inline void BindAttribute() {
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTLX), (void*)(0));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTLX), (void*)(12));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(VertexTLX), (void*)(12 + 8));
	}
	static inline void UnbindAttribute() {
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
	}
};

class DataBuffer {
public:
	DataBuffer(GLushort type);
	~DataBuffer();

	void Create(size_t size, size_t stride);
	void Release();

	GLuint GetBuffer() { return id_; }
	GLushort GetType() { return type_; }
	size_t GetSize() { return size_; }
	
	inline bool Bind() { 
		if (id_) 
			glBindBuffer(type_, id_); 
		return id_ != 0;
	}
	inline void Unbind() {
		glBindBuffer(type_, 0);
	}
	inline void LoadData(LPVOID data, size_t dataSize, GLushort usage) {
		if (id_) {
			Bind();
			glBufferData(type_, std::min(dataSize, size_) * stride_, data, usage);
		}
	}
private:
	GLuint id_;
	GLushort type_;
	size_t size_;
	size_t stride_;
};