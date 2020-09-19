#include "pch.h"
#include "Utility.hpp"
#include "Input.hpp"

//*******************************************************************
//InputManager
//*******************************************************************
InputManager* InputManager::base_ = nullptr;
InputManager::InputManager() {
}
InputManager::~InputManager() {
}

void InputManager::Initialize() {
	if (base_) throw EngineError("InputManager already initialized.");
	base_ = this;

	mapVirtualKey_.insert(std::make_pair(VK_LEFT, VirtualKey::Left));
	mapVirtualKey_.insert(std::make_pair(VK_UP, VirtualKey::Up));
	mapVirtualKey_.insert(std::make_pair(VK_RIGHT, VirtualKey::Right));
	mapVirtualKey_.insert(std::make_pair(VK_DOWN, VirtualKey::Down));

	mapVirtualKey_.insert(std::make_pair(0x5A, VirtualKey::Shot));
	mapVirtualKey_.insert(std::make_pair(0x58, VirtualKey::Spell));
	mapVirtualKey_.insert(std::make_pair(VK_LSHIFT, VirtualKey::Focus));

	mapVirtualKey_.insert(std::make_pair(0x5A, VirtualKey::Ok));
	mapVirtualKey_.insert(std::make_pair(0x58, VirtualKey::Cancel));
	mapVirtualKey_.insert(std::make_pair(VK_ESCAPE, VirtualKey::Pause));

	//------------------------------------------------------------------------------------

	mapState_.insert(std::make_pair(VirtualKey::Left, KeyState::Free));
	mapState_.insert(std::make_pair(VirtualKey::Up, KeyState::Free));
	mapState_.insert(std::make_pair(VirtualKey::Right, KeyState::Free));
	mapState_.insert(std::make_pair(VirtualKey::Down, KeyState::Free));

	mapState_.insert(std::make_pair(VirtualKey::Shot, KeyState::Free));
	mapState_.insert(std::make_pair(VirtualKey::Spell, KeyState::Free));
	mapState_.insert(std::make_pair(VirtualKey::Focus, KeyState::Free));

	mapState_.insert(std::make_pair(VirtualKey::Ok, KeyState::Free));
	mapState_.insert(std::make_pair(VirtualKey::Cancel, KeyState::Free));
	mapState_.insert(std::make_pair(VirtualKey::Pause, KeyState::Free));
}
void InputManager::Release() {
}

void InputManager::Update() {
	for (auto itrKey = mapVirtualKey_.begin(); itrKey != mapVirtualKey_.end(); ++itrKey) {
		bool stateRaw = ::GetKeyState(itrKey->first) & 0x8000;
		auto itrVk = mapState_.find(itrKey->second);
		if (itrVk != mapState_.end()) {
			if (stateRaw)	//Key is pressed
				itrVk->second = itrVk->second == KeyState::Free ? KeyState::Push : KeyState::Hold;
			else
				itrVk->second = itrVk->second == KeyState::Hold ? KeyState::Pull : KeyState::Free;
		}
	}
}

KeyState InputManager::GetKeyState(VirtualKey key) {
	auto itrFind = mapState_.find(key);
	if (itrFind == mapState_.end()) return KeyState::Free;
	return itrFind->second;
}