#pragma once
#include "PlaceholderExpansion.h"

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

	bool IsValid() const
	{
		return m_Handle && *m_Handle;
	}

	operator std::shared_ptr<bool>() const
	{
		return m_Handle;
	}

	bool operator==(const AkDelegateHandle& rhs) const
	{
		return m_Handle == rhs.m_Handle;
	}

private:
	void Initialize() { m_Handle = std::make_shared<bool>(true); }
	void Invalidate() { if (m_Handle) *m_Handle = false; }
	std::shared_ptr<bool> m_Handle = nullptr;
};

namespace std
{
	template <>
	struct hash<AkDelegateHandle>
	{
		size_t operator()(const AkDelegateHandle& handle) const
		{
			std::hash<std::shared_ptr<bool>> hasher;
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
		AkDelegateHandle handle;
		handle.Initialize();

		auto function = std::bind(callback, static_cast<OwnerClass*>(owner));
		m_Subscribers[handle] = function;
		return std::move(handle);
	}

	AkDelegateHandle Add(const std::function<void()>& function)
	{
		AkDelegateHandle handle;
		handle.Initialize();

		m_Subscribers[handle] = function;
		return std::move(handle);
	}

	void Broadcast()
	{
		for (auto& [handle, callback] : m_Subscribers)
			if (handle.IsValid())
				callback();
	}

	void Remove(AkDelegateHandle& handle)
	{
		auto found = m_Subscribers.find(handle);
		if (found != m_Subscribers.end())
		{
			handle.Invalidate();
			m_Subscribers.erase(found);
		}
	}

	void RemoveAll()
	{
		for (auto& [handle, callback] : m_Subscribers)
			const_cast<AkDelegateHandle&>(handle).Invalidate();

		m_Subscribers.clear();
	}

private:
	std::unordered_map<AkDelegateHandle, std::function<void()>> m_Subscribers;
};

template<typename... Types>
class AkCustomDelegate
{
public:
	template<typename OwnerClass, typename... Types>
	AkDelegateHandle Add(void* owner, void(OwnerClass::* callback)(Types... Args))
	{
		AkDelegateHandle handle;
		handle.Initialize();

		auto function = AutomaticPlaceholderExpansionBind(owner, callback);
		m_Subscribers[handle] = function;
		return std::move(handle);
	}

	AkDelegateHandle Add(const std::function<void(Types... Args)>& function)
	{
		AkDelegateHandle handle;
		handle.Initialize();

		m_Subscribers[handle] = function;
		return std::move(handle);
	}

	void Broadcast(Types... Args)
	{
		for (auto& [handle, callback] : m_Subscribers)
		{
			if (handle.IsValid())
				callback(Args...);
		}
	}

	void Remove(AkDelegateHandle& handle)
	{
		auto found = m_Subscribers.find(handle);
		if (found != m_Subscribers.end())
		{
			handle.Invalidate();
			m_Subscribers.erase(found);
		}
	}

	void RemoveAll()
	{
		for (auto& [handle, callback] : m_Subscribers)
			const_cast<AkDelegateHandle&>(handle).Invalidate();

		m_Subscribers.clear();
	}

private:
	std::unordered_map<AkDelegateHandle, std::function<void(Types... Args)>> m_Subscribers;
};