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
private:
	std::multimap<uint16_t, VirtualKey> mapVirtualKey_;
	std::map<VirtualKey, KeyState> mapState_;
public:
	InputManager();
	~InputManager();

	static InputManager* const GetBase() { return base_; }

	void Initialize();
	void Release();

	void Update();

	KeyState GetKeyState(VirtualKey key);
};