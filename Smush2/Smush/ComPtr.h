// Copyright (c) Jérémy Ansel 2014, 2019
// Licensed under the MIT license. See LICENSE.txt

#pragma once

template<class T>
class ComPtr
{
private:
	T* _ptr;

public:
	ComPtr()
	{
		this->_ptr = nullptr;
	}

	ComPtr(T* ptr)
	{
		this->_ptr = ptr;
	}

	~ComPtr()
	{
		this->Release();
	}

	operator T*() const
	{
		return this->_ptr;
	}

	operator bool() const
	{
		return this->_ptr != nullptr;
	}

	T* operator ->()
	{
		return this->_ptr;
	}

	T** operator &()
	{
		this->Release();
		return &this->_ptr;
	}

	void Release()
	{
		if (this->_ptr != nullptr)
		{
			this->_ptr->Release();
			this->_ptr = nullptr;
		}
	}

	T* Get() const
	{
		return this->_ptr;
	}

	T** GetAddressOf() const
	{
		return &this->_ptr;
	}

	template<class U>
	HRESULT As(U** ptr)
	{
		return this->_ptr->QueryInterface(IID_PPV_ARGS(ptr));
	}
};
