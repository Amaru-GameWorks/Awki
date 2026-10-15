#pragma once
#include "JobSystem.h"
#include "Core/Assert.h"

#include <queue>

class AkTicketMachine
{
public:
	AkTicketMachine(AkThreadType targetThread)
		: m_TargetThread(targetThread)
	{ }

	struct Ticket
	{
		AkTicketMachine& ticketMachine;
		size_t turn;

		bool await_ready() const noexcept 
		{ 
			return ticketMachine.TurnReady(turn);
		};

		bool await_suspend(std::coroutine_handle<> handle) const noexcept
		{
			if (ticketMachine.TurnReady(turn))
				return false;

			ticketMachine.Schedule(handle);
			return true;
		}

		constexpr void await_resume() const noexcept {}

		void Done()
		{
			ticketMachine.Advance();
		}
	};

	Ticket GetTicket()
	{
		return Ticket{ *this, m_Counter.fetch_add(1) };
	}

private:
	std::atomic_size_t m_Turn = 0;
	std::atomic_size_t m_Counter = 0;
	AkThreadType m_TargetThread = {};

	bool TurnReady(size_t turn)
	{
		return m_Turn.load(std::memory_order_acquire) == turn;
	}

	void Advance()
	{
		++m_Turn;
	}

	void Schedule(std::coroutine_handle<> handle)
	{
		AkJobSystem::Schedule(m_TargetThread, handle);
	}
};

class AkSemaphore
{
public:
	AkSemaphore(AkThreadType targetThread)
		: m_TargetThread(targetThread)
	{ }

	AkSemaphore(AkThreadType targetThread, size_t initialCount)
		: m_TargetThread(targetThread)
		, m_Count(initialCount)
		, m_MaxCount(initialCount)
	{ }

	void Signal()
	{
		std::coroutine_handle<> job = nullptr;
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			if (m_Waiters.empty())
			{
				AkAssert(m_Count <= m_MaxCount, "Signal called too many times!");
				++m_Count;
			}
			else
			{
				job = m_Waiters.front();
				m_Waiters.pop();
			}
		}

		if (job)
			AkJobSystem::Schedule(m_TargetThread, job);
	}

	void Reset(size_t newCount)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		AkAssert(m_Waiters.empty(), "There are still waiters left in the semaphore");

		m_Count = newCount;
		m_MaxCount = newCount;
	}

	struct Awaiter 
	{
		AkSemaphore& semaphore;
		bool await_ready() const noexcept { return semaphore.Ready(); }
		bool await_suspend(std::coroutine_handle<> handle) noexcept { return semaphore.Suspend(handle); }
		constexpr void await_resume() noexcept {}
	};

	Awaiter Wait()
	{
		return Awaiter{ *this };
	}

private:
	size_t m_Count = 0;
	size_t m_MaxCount = 0;
	AkThreadType m_TargetThread = {};

	std::mutex m_Mutex;
	std::queue<std::coroutine_handle<>> m_Waiters;

	bool Ready() noexcept
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		
		if (m_Count > 0) 
		{
			--m_Count;
			return true;
		}

		return false;
	}

	bool Suspend(std::coroutine_handle<> handle) noexcept 
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		
		if (m_Count > 0) 
		{
			--m_Count;
			return false;
		}

		m_Waiters.push(handle);
		return true;
	}
};

class AkCountdownLatch
{
public:
	AkCountdownLatch(AkThreadType targetThread)
		: m_TargetThread(targetThread)
	{ }

	AkCountdownLatch(AkThreadType targetThread, size_t initialCount)
		: m_TargetThread(targetThread)
		, m_Count(initialCount)
	{ }

	void Signal()
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		m_Count -= 1;

		if (m_Count == 0)
		{
			for(auto waiter : m_Waiters)
				AkJobSystem::Schedule(m_TargetThread, waiter);
			m_Waiters.clear();

			m_Conditional.notify_all();
		}
	}

	void Reset(size_t newCount)
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		AkAssert(m_Waiters.empty(), "There are still waiters left in the semaphore");
		m_Count = newCount;
	}

	struct Awaiter
	{
		AkCountdownLatch& countdown;
		bool await_ready() const noexcept { return countdown.Ready(); }
		bool await_suspend(std::coroutine_handle<> handle) noexcept { return countdown.Suspend(handle); }
		constexpr void await_resume() noexcept {}
	};

	Awaiter Wait()
	{
		return Awaiter{ *this };
	}

	void WaitBlocking()
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_Conditional.wait(lock, [&]()
		{
			return m_Count == 0;
		});
	}

private:
	size_t m_Count = 0;
	AkThreadType m_TargetThread = {};

	std::mutex m_Mutex;
	std::condition_variable m_Conditional;
	std::vector<std::coroutine_handle<>> m_Waiters;

	bool Ready() noexcept
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		return m_Count == 0;
	}

	bool Suspend(std::coroutine_handle<> handle) noexcept
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		if (m_Count == 0)
			return false;

		m_Waiters.push_back(handle);
		return true;
	}
};