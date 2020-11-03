#include "pch.h"
#include "Utility.hpp"
#include "Input.hpp"

//*******************************************************************
//InputManager
//*******************************************************************
InputManager* InputManager::base_ = nullptr;
InputManager::InputManager() {
	hWnd_ = nullptr;

	pDirectInput_ = nullptr;
	pDeviceKeyboard_ = nullptr;
	
	//ZeroMemory(stateKey_, sizeof(stateKey_));
}
InputManager::~InputManager() {
}

void InputManager::Initialize(HINSTANCE hInst, HWND hWnd) {
	if (base_) throw EngineError("InputManager already initialized.");
	base_ = this;

	hWnd_ = hWnd;

	auto WrapDXErr = [&](HRESULT hr, const std::string& msg) {
		if (FAILED(hr)) {
			std::string err = StringUtility::Format("Input: %s\n\t%s",
				msg.c_str(), ErrorUtility::StringFromHResult(hr).c_str());
			throw EngineError(err);
		}
	};

	HRESULT hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(LPVOID*)&pDirectInput_, nullptr);
	WrapDXErr(hr, "DirectInput8Create failure.");

	//Initialize keyboard
	{
		hr = pDirectInput_->CreateDevice(GUID_SysKeyboard, &pDeviceKeyboard_, nullptr);
		WrapDXErr(hr, "Failed to create keyboard object.");

		hr = pDeviceKeyboard_->SetDataFormat(&c_dfDIKeyboard);
		WrapDXErr(hr, "Failed to set keyboard data format.");

		hr = pDeviceKeyboard_->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
		WrapDXErr(hr, "Failed to set keyboard cooperative level.");

		hr = pDeviceKeyboard_->Acquire();
		WrapDXErr(hr, "Failed to acquire keyboard object.");
	}

	//Initialize pad
	{
		hr = pDirectInput_->EnumDevices(DI8DEVCLASS_GAMECTRL, 
			(LPDIENUMDEVICESCALLBACK)_EnumPadCallback, this, DIEDFL_ATTACHEDONLY);
		//WrapDXErr(hr, "Failed to connect to a joypad.");

		listStatePad_.resize(pListDevicePad_.size());
	}

	{
		mapRelationKeyboard_.insert(std::make_pair(DIK_LEFT, VirtualKey::Left));
		mapRelationKeyboard_.insert(std::make_pair(DIK_UP, VirtualKey::Up));
		mapRelationKeyboard_.insert(std::make_pair(DIK_RIGHT, VirtualKey::Right));
		mapRelationKeyboard_.insert(std::make_pair(DIK_DOWN, VirtualKey::Down));

		mapRelationKeyboard_.insert(std::make_pair(DIK_Z, VirtualKey::Shot));
		mapRelationKeyboard_.insert(std::make_pair(DIK_X, VirtualKey::Spell));
		mapRelationKeyboard_.insert(std::make_pair(DIK_LSHIFT, VirtualKey::Focus));

		mapRelationKeyboard_.insert(std::make_pair(DIK_Z, VirtualKey::Ok));
		mapRelationKeyboard_.insert(std::make_pair(DIK_X, VirtualKey::Cancel));
		mapRelationKeyboard_.insert(std::make_pair(DIK_ESCAPE, VirtualKey::Pause));

		//----------------------------------------------------------------
		//XBOX 360 Controller mappings

		mapRelationPad_Button_.insert(std::make_pair(0, VirtualKey::Shot));		//A
		mapRelationPad_Button_.insert(std::make_pair(4, VirtualKey::Spell));	//L1
		mapRelationPad_Button_.insert(std::make_pair(5, VirtualKey::Focus));	//R1

		mapRelationPad_Button_.insert(std::make_pair(0, VirtualKey::Ok));		//A
		mapRelationPad_Button_.insert(std::make_pair(4, VirtualKey::Cancel));	//L1
		mapRelationPad_Button_.insert(std::make_pair(2, VirtualKey::Pause));	//X
		mapRelationPad_Button_.insert(std::make_pair(7, VirtualKey::Pause));	//Start

		//------------------------------------------------------------------------------------

		mapState_[VirtualKey::Left] = std::make_pair(false, KeyState::Free);
		mapState_[VirtualKey::Up] = std::make_pair(false, KeyState::Free);
		mapState_[VirtualKey::Right] = std::make_pair(false, KeyState::Free);
		mapState_[VirtualKey::Down] = std::make_pair(false, KeyState::Free);

		mapState_[VirtualKey::Shot] = std::make_pair(false, KeyState::Free);
		mapState_[VirtualKey::Spell] = std::make_pair(false, KeyState::Free);
		mapState_[VirtualKey::Focus] = std::make_pair(false, KeyState::Free);

		mapState_[VirtualKey::Ok] = std::make_pair(false, KeyState::Free);
		mapState_[VirtualKey::Cancel] = std::make_pair(false, KeyState::Free);
		mapState_[VirtualKey::Pause] = std::make_pair(false, KeyState::Free);
	}
}
void InputManager::Release() {
	if (pDeviceKeyboard_) {
		pDeviceKeyboard_->Unacquire();
		ptr_release(pDeviceKeyboard_);
	}

	for (auto& pPadDevice : pListDevicePad_) {
		if (pPadDevice) {
			pPadDevice->Unacquire();
			ptr_release(pPadDevice);
		}
	}
	pListDevicePad_.clear();

	ptr_release(pDirectInput_);
}

BOOL InputManager::_EnumPadCallback(LPDIDEVICEINSTANCE lpddi, LPVOID pvRef) {
	InputManager* input = (InputManager*)pvRef;

	IDirectInputDevice8* pPadDevice = nullptr;

	HRESULT hr = input->pDirectInput_->CreateDevice(lpddi->guidInstance, &pPadDevice, nullptr);
	if (FAILED(hr))
		return DIENUM_CONTINUE;		//Failed to create device

	hr = pPadDevice->SetDataFormat(&c_dfDIJoystick);
	if (FAILED(hr)) {
		ptr_release(pPadDevice);
		return DIENUM_CONTINUE;		//Failed to set data format
	}
	hr = pPadDevice->SetCooperativeLevel(input->hWnd_, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	if (FAILED(hr)) {
		ptr_release(pPadDevice);
		return DIENUM_CONTINUE;		//Failed to set cooperative level
	}

	//Set the axis range
	{
		DIPROPRANGE diprg;
		diprg.diph.dwSize = sizeof(DIPROPRANGE);
		diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		diprg.diph.dwObj = 0;
		diprg.diph.dwHow = DIPH_DEVICE;
		diprg.lMin = -1000;
		diprg.lMax = +1000;

		hr = pPadDevice->SetProperty(DIPROP_RANGE, &diprg.diph);
		if (FAILED(hr)) {
			ptr_release(pPadDevice);
			return DIENUM_CONTINUE;		//Failed to set axis input range
		}
	}

	//Set the axis deadzone
	{
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = 1000;	//10% deadzone

		hr = pPadDevice->SetProperty(DIPROP_DEADZONE, &dipdw.diph);
		if (FAILED(hr)) {
			ptr_release(pPadDevice);
			return DIENUM_CONTINUE;		//Failed to set axis input deadzone
		}
	}

	hr = pPadDevice->Acquire();
	if (FAILED(hr)) {
		ptr_release(pPadDevice);
		return DIENUM_CONTINUE;		//Object acquisition failed
	}

	input->pListDevicePad_.push_back(pPadDevice);

	return DIENUM_CONTINUE;
}

void InputManager::_SetStateSub(KeyState* pState, bool bPress) {
	if (bPress)		//Key is pressed
		*pState = (*pState) == KeyState::Free ? KeyState::Push : KeyState::Hold;
	else
		*pState = (*pState) == KeyState::Hold ? KeyState::Pull : KeyState::Free;
}

void InputManager::Update() {
	//Update raw keys
	{
		//Update keyboard
		{
			pDeviceKeyboard_->Poll();
			HRESULT hr = pDeviceKeyboard_->GetDeviceState(MAX_KEYBOARD_BUTTON, stateKeyboard_);
			if (hr == DIERR_INPUTLOST)
				pDeviceKeyboard_->Acquire();
		}

		//Update pads
		if (pListDevicePad_.size() > 0) {
			for (size_t iPad = 0; iPad < pListDevicePad_.size(); ++iPad) {
				IDirectInputDevice8* pPadDevice = pListDevicePad_[iPad];
				if (pPadDevice == nullptr)
					continue;

				pPadDevice->Poll();
				HRESULT hr = pPadDevice->GetDeviceState(sizeof(DIJOYSTATE), &listStatePad_[iPad]);
				if (hr == DIERR_INPUTLOST)
					pPadDevice->Acquire();
			}
		}
	}

	//Update virtual keys
	for (auto itrKey = mapRelationKeyboard_.begin(); itrKey != mapRelationKeyboard_.end(); ++itrKey) {
		auto itrVk = mapState_.find(itrKey->second);
		if (itrVk != mapState_.end())
			itrVk->second.first = stateKeyboard_[itrKey->first];
	}

	//Update virtual keys (joypad)
	if (DIJOYSTATE* pStatePad = GetPadState(0)) {
		//Update D-Pads
		if (pStatePad->rgdwPOV[0] != (DWORD)0xffffffff) {
			auto itrKeyLeft = mapState_.find(VirtualKey::Left);
			auto itrKeyRight = mapState_.find(VirtualKey::Right);
			auto itrKeyUp = mapState_.find(VirtualKey::Up);
			auto itrKeyDown = mapState_.find(VirtualKey::Down);

			DWORD powValueDeg = pStatePad->rgdwPOV[0] / 100UL;

			itrKeyUp->second.first |= powValueDeg == 0;
			itrKeyRight->second.first |= powValueDeg == 90;
			itrKeyDown->second.first |= powValueDeg == 180;
			itrKeyLeft->second.first |= powValueDeg == 270;
		}

		//Update buttons
		for (auto itrKey = mapRelationPad_Button_.begin(); itrKey != mapRelationPad_Button_.end(); ++itrKey) {
			auto itrVk = mapState_.find(itrKey->second);
			if (itrVk != mapState_.end())
				itrVk->second.first |= (bool)pStatePad->rgbButtons[itrKey->first];
		}
	}

	for (auto itr = mapState_.begin(); itr != mapState_.end(); ++itr) {
		_SetStateSub(&itr->second.second, itr->second.first);
		itr->second.first = false;
	}
}

KeyState InputManager::GetKeyState(VirtualKey key) {
	auto itrFind = mapState_.find(key);
	if (itrFind == mapState_.end()) return KeyState::Free;
	return itrFind->second.second;
}