#pragma once
#include "../../pch.h"

#include "GConstant.hpp"

class ScriptSoundLibrary {
	static ScriptSoundLibrary* base_;
private:
	std::map<std::string, shared_ptr<SoundResource>> mapSound_;
public:
	ScriptSoundLibrary();

	static ScriptSoundLibrary* const GetBase() { return base_; }

	//Volume from 0 to 100
	void PlaySE(const std::string& id, double volume = 100);
};