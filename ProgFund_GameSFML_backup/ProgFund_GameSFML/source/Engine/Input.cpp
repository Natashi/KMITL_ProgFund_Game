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

	mapVirtualKey_.insert(std::make_pair(sf::Keyboard::Left, VirtualKey::Left));
	mapVirtualKey_.insert(std::make_pair(sf::Keyboard::Up, VirtualKey::Up));
	mapVirtualKey_.insert(std::make_pair(sf::Keyboard::Right, VirtualKey::Right));
	mapVirtualKey_.insert(std::make_pair(sf::Keyboard::Down, VirtualKey::Down));

	mapVirtualKey_.insert(std::make_pair(sf::Keyboard::Z, VirtualKey::Shot));
	mapVirtualKey_.insert(std::make_pair(sf::Keyboard::X, VirtualKey::Spell));
	mapVirtualKey_.insert(std::make_pair(sf::Keyboard::LShift, VirtualKey::Focus));

	mapVirtualKey_.insert(std::make_pair(sf::Keyboard::Z, VirtualKey::Ok));
	mapVirtualKey_.insert(std::make_pair(sf::Keyboard::X, VirtualKey::Cancel));
	mapVirtualKey_.insert(std::make_pair(sf::Keyboard::Escape, VirtualKey::Pause));

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

void InputManager::Update() {
	for (auto itrKey = mapVirtualKey_.begin(); itrKey != mapVirtualKey_.end(); ++itrKey) {
		bool stateRaw = sf::Keyboard::isKeyPressed(itrKey->first);
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