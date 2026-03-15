#pragma once
#include "PlaceholderExpansion.h"

#include <atomic>
#include <memory>
#include <functional>
#include <unordered_map>
#define AK_DECLARE_CUSTOM_DELEGATE(DelegateType, ...) using DelegateType = AkCustomDelegate<__VA_ARGS__>;

class AkDelegateHandle
{
	template<typename... Types>
	friend class AkCustomDelegate;
	friend class AkSimpleDelegate;

public:
	AkDelegateHandle() = default;
	AkDelegateHandle(const AkDelegateHandle& other) { m_Handle = other.m_Handle; }

	operator uint32_t() const
	{
		return m_Handle;
	}

	bool operator==(const AkDelegateHandle& rhs) const
	{
		return m_Handle == rhs.m_Handle;
	}

private:
	AkDelegateHandle(const uint32_t& handle) { m_Handle = handle; }
	uint32_t m_Handle;
};

namespace std
{
	template <>
	struct hash<AkDelegateHandle>
	{
		size_t operator()(const AkDelegateHandle& handle) const
		{
			std::hash<uint32_t> hasher;
			return hasher(handle);
		}
	};
}

class AkSimpleDelegate
{
public:
	~AkSimpleDelegate()
	{
		RemoveAll();
	}

	template<typename OwnerClass>
	AkDelegateHandle Add(void* owner, void(OwnerClass::* callback)())
	{
		AkDelegateHandle handle = m_Counter.fetch_add(1);
		auto function = std::bind(callback, static_cast<OwnerClass*>(owner));
		m_Subscribers[handle] = function;
		return handle;
	}

	AkDelegateHandle Add(const std::function<void()>& function)
	{
		AkDelegateHandle handle = m_Counter.fetch_add(1);
		m_Subscribers[handle] = function;
		return handle;
	}

	void Broadcast()
	{
		for (auto& [handle, callback] : m_Subscribers)
			callback();
	}

	void Remove(AkDelegateHandle& handle)
	{
		auto found = m_Subscribers.find(handle);
		if (found != m_Subscribers.end())
			m_Subscribers.erase(found);
	}

	void RemoveAll()
	{
		m_Subscribers.clear();
	}

private:
	std::atomic_uint32_t m_Counter;
	std::unordered_map<AkDelegateHandle, std::function<void()>> m_Subscribers;
};

template<typename... Types>
class AkCustomDelegate
{
public:
	template<typename OwnerClass>
	AkDelegateHandle Add(void* owner, void(OwnerClass::* callback)(Types... Args))
	{
		AkDelegateHandle handle = m_Counter.fetch_add(1);
		auto function = AutomaticPlaceholderExpansionBind(owner, callback);
		m_Subscribers[handle] = function;
		return handle;
	}

	AkDelegateHandle Add(const std::function<void(Types... Args)>& function)
	{
		AkDelegateHandle handle = m_Counter.fetch_add(1);
		m_Subscribers[handle] = function;
		return handle;
	}

	void Broadcast(Types... Args)
	{
		for (auto& [handle, callback] : m_Subscribers)
			callback(Args...);
	}

	void Remove(AkDelegateHandle& handle)
	{
		auto found = m_Subscribers.find(handle);
		if (found != m_Subscribers.end())
			m_Subscribers.erase(found);
	}

	void RemoveAll()
	{
		m_Subscribers.clear();
	}

private:
	std::atomic_uint32_t m_Counter;
	std::unordered_map<AkDelegateHandle, std::function<void(Types... Args)>> m_Subscribers;
};