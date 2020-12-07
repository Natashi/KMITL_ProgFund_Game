#pragma once

#include "../../pch.h"

template<typename T> static constexpr inline void ptr_delete(T*& ptr) {
	if (ptr) delete ptr;
	ptr = nullptr;
}
template<typename T> static constexpr inline void ptr_delete_scalar(T*& ptr) {
	if (ptr) delete[] ptr;
	ptr = nullptr;
}
template<typename T> static constexpr inline void ptr_release(T*& ptr) {
	if (ptr) ptr->Release();
	ptr = nullptr;
}
using std::shared_ptr;
using std::weak_ptr;

#define LOCK_WEAK(pn) if (auto p_##pn = pn.lock())
#define LOCK_WEAK2(pn, rn) if (auto rn = pn.lock())

template<class T, class U>
weak_ptr<T> static_pointer_cast_weak(const weak_ptr<U>& r) {
	return std::static_pointer_cast<T>(r.lock());
}

/*
//-------------------------------------------------------------------------------------

//Pointer reference counter
class __declspec(novtable) _Ptr_RefCounter_Base {
private:
	uint64_t countUse_ = 1ui64;
	uint64_t countWeak_ = 1ui64;
private:
	virtual void _Release() noexcept = 0;
	virtual void _DeleteSelf() noexcept = 0;
public:
	_Ptr_RefCounter_Base(const _Ptr_RefCounter_Base&) = delete;
	_Ptr_RefCounter_Base& operator=(const _Ptr_RefCounter_Base&) = delete;

	virtual ~_Ptr_RefCounter_Base() noexcept {}

	//Increments use count if it's not zero, return true if successful
	bool AddRef_NotNull() noexcept {
		if (countUse_ != 0) {
			++countUse_;
			return true;
		}
		return false;
	}

	//Increments use count
	void AddRef() noexcept { 
		++countUse_;
	}
	//Increments weak ref count
	void AddRefWeak() noexcept {
		++countWeak_;
	}

	//Decrements use count
	void RemoveRef() noexcept {
		if ((--countUse_) == 0) {
			_Release();
			RemoveRefWeak();
		}
	}
	//Decrements weak ref count
	void RemoveRefWeak() noexcept {
		if ((--countWeak_) == 0) {
			_DeleteSelf();
		}
	}

	uint64_t GetReferenceCount() const noexcept { return countUse_; }
	virtual void* GetDeleter(const type_info&) const noexcept { return nullptr; }
};

template <class T>
class _Ptr_RefCounter : public _Ptr_RefCounter_Base { // handle reference counting for pointer without deleter
private:
	T* ptr_;
public:
	explicit _Ptr_RefCounter(T* p) : _Ptr_RefCounter_Base(), ptr_(p) {}
private:
	virtual void _Release() noexcept override { delete ptr_; }
	virtual void _DeleteSelf() noexcept override { delete this; }
};

//Base pointer class for shared_ptr_single
template<class T>
class _PtrBase_NonAtomic {
public:
	using element_type = std::remove_extent_t<T>;
private:
	element_type* pointer_ { nullptr };
	_Ptr_RefCounter_Base* refCount_ { nullptr };

	template <class D, class S>
	friend D* get_deleter(const shared_ptr<S>& _Sx) noexcept;
public:
	using element_type = std::remove_extent_t<T>;

	_NODISCARD uint64_t use_count() const noexcept {
		return refCount_ ? refCount_->GetReferenceCount() : 0;
	}

	template <class T2>
	_NODISCARD bool owner_before(const _PtrBase_NonAtomic<T2>& other) const noexcept {
		return refCount_ < other.refCount_;
	}

	_PtrBase_NonAtomic(const _PtrBase_NonAtomic&) = delete;
	_PtrBase_NonAtomic& operator=(const _PtrBase_NonAtomic&) = delete;
};

//Non-atomic shared_ptr
template <class T>
class shared_ptr_single : public _PtrBase_NonAtomic<T> {
private:
	using _Mybase = _PtrBase_NonAtomic<T>;
public:
	using typename _Mybase::element_type;
	using weak_type = weak_ptr_single<T>;
public:
	constexpr shared_ptr() noexcept = default;
	constexpr shared_ptr(nullptr_t) noexcept {}

	template <class U,
		std::enable_if_t<std::conjunction_v<std::conditional_t<std::is_array_v<T>, std::_Can_array_delete<U>, 
			std::_Can_scalar_delete<U>>,
		std::_SP_convertible<U, T>>, int> = 0>
	explicit shared_ptr(U * nptr) {
		if constexpr (std::is_array_v<T>) {
			_Setpd(_Px, default_delete<_Ux[]>{});
		}
		else {
			_Temporary_owner<_Ux> _Owner(_Px);
			_Set_ptr_rep_and_enable_shared(_Owner._Ptr, new _Ref_count<_Ux>(_Owner._Ptr));
			_Owner._Ptr = nullptr;
		}
	}
};
*/