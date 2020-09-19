#include "pch.h"
#include "Vertex.hpp"

//*******************************************************************
//DataBuffer
//*******************************************************************
DataBuffer::DataBuffer(GLushort type) {
	id_ = 0;
	type_ = type;
	size_ = 0;
	stride_ = 0;
}
DataBuffer::~DataBuffer() {
	Release();
}
void DataBuffer::Create(size_t size, size_t stride) {
	Release();
	glGenBuffers(1, &id_);
	size_ = size;
	stride_ = stride;
}
void DataBuffer::Release() {
	if (id_) {
		glDeleteBuffers(1, &id_);
		id_ = 0;
	}
	size_ = 0;
	stride_ = 0;
}