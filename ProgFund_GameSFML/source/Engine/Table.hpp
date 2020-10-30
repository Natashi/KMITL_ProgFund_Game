#pragma once

#include "../../pch.h"

class ManagedValue {
public:
	std::vector<byte> data;
public:
	ManagedValue() {};
	ManagedValue(int i) { SetAs(i); }
	ManagedValue(float f) { SetAs(f); }
	ManagedValue(const std::string& f) { 
		SetAs(f.data(), f.size() * sizeof(char));
	}
	~ManagedValue() {};

	template<typename T>
	ManagedValue Create(const T& value) {
		return Create(&value, sizeof(value));
	}
	template<typename T>
	ManagedValue Create(const T* ptr, size_t size) {
		ManagedValue res;
		res.SetAs(ptr, size);
		return res;
	}

	template<typename T>
	T GetAs() const {
		return *(T*)data.data();
	}

	template<typename T>
	void SetAs(const T& value) {
		SetAs(&value, sizeof(value));
	}
	template<typename T>
	void SetAs(const T* ptr, size_t size) {
		data.resize(size);
		memcpy(data.data(), ptr, size);
	}
};

class CommonDataManager {
	static CommonDataManager* base_;
private:
	std::map<std::string, ManagedValue> mapValue_;
public:
	CommonDataManager();
	~CommonDataManager();

	static CommonDataManager* GetBase() { return base_; }

	void Initialize();
	void Release();

	ManagedValue* GetValue(const std::string& key);
	void SetValue(const std::string& key, const ManagedValue& value);
	void DeleteValue(const std::string& key);
};