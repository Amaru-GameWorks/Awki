#pragma once
#include <cstdint>

template<typename T, size_t Size>
class ForwardStorage
{
public:
	ForwardStorage()
	{
		static_assert(Size >= sizeof(T), "Size mismatch in ForwardDeclaredStrage");
		new (this) T();
	}

	~ForwardStorage()
	{
		this->operator->()->~T();
	}

	T* operator->()
	{
		return reinterpret_cast<T*>(this);
	}

	const T* operator->() const
	{
		return reinterpret_cast<const T*>(this);
	}

private:
	uint8_t m_Storage[Size];
};