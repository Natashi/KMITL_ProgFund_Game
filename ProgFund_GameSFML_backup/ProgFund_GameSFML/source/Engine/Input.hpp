#pragma once
#include "../../pch.h"

enum class KeyState : byte {
	Free,
	Push,
	Hold,
	Pull,
};
enum class VirtualKey : byte {
	Left,
	Up,
	Right,
	Down,
	Shot,
	Spell,
	Focus,
	Ok,
	Cancel,
	Pause,
};

class InputManager {
	static InputManager* base_;
public:
	InputManager();
	~InputManager();

	static InputManager* const GetBase() { return base_; }

	void Initialize();

	void Update();

	KeyState GetKeyState(VirtualKey key);
private:
	std::multimap<sf::Keyboard::Key, VirtualKey> mapVirtualKey_;
	std::map<VirtualKey, KeyState> mapState_;
};