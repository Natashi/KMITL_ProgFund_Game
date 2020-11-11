#pragma once

#define CD3DXVECTOR2 const D3DXVECTOR2&
#define CD3DXVECTOR3 const D3DXVECTOR3&

#define GET_INSTANCE(_type, _name) _type* _name = _type::GetBase();

class DxResourceManagerBase {
public:
	virtual ~DxResourceManagerBase() {}

	virtual void OnLostDevice() {}
	virtual void OnRestoreDevice() {}
};