#include "pch.h"
#include "Table.hpp"
#include "Utility.hpp"

//*******************************************************************
//CommonDataManager
//*******************************************************************
CommonDataManager* CommonDataManager::base_ = nullptr;
CommonDataManager::CommonDataManager() {
}
CommonDataManager::~CommonDataManager() {
}

void CommonDataManager::Initialize() {
	if (base_) throw EngineError("CommonDataManager already initialized.");
	base_ = this;
}
void CommonDataManager::Release() {

}

ManagedValue* CommonDataManager::GetValue(const std::string& key) {
	auto itr = mapValue_.find(key);
	if (itr != mapValue_.end()) return &itr->second;
	return nullptr;
}
void CommonDataManager::SetValue(const std::string& key, const ManagedValue& value) {
	mapValue_[key] = value;
}
void CommonDataManager::DeleteValue(const std::string& key) {
	mapValue_.erase(key);
}