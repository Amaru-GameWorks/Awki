#pragma once
#include "JobSystem.h"

#include <queue>

class AkTicketMachine
{
public:
	AkTicketMachine(AkThreadType targetThread)
		: m_TargetThread(targetThread)
	{ }

	struct Awaiter
	{
		AkTicketMachine& ticketMachine;
		size_t turn;

		bool await_ready() const noexcept 
		{ 
			return ticketMachine.TurnReady(turn);
		};

		void await_suspend(std::coroutine_handle<> handle) const noexcept
		{
			if (ticketMachine.TurnReady(turn))
				return;

			ticketMachine.Schedule(handle);
		}

		constexpr void await_resume() const noexcept {}
	};

	Awaiter WaitForTurn()
	{
		return Awaiter{ *this, m_Counter.fetch_add(1) };
	}

private:
	std::atomic_size_t m_Turn;
	std::atomic_size_t m_Counter;
	AkThreadType m_TargetThread;

	bool TurnReady(size_t turn)
	{
		return m_Turn.compare_exchange_strong(turn, turn + 1);
	}

	void Schedule(std::coroutine_handle<> handle)
	{
		AkJobSystem::Schedule(m_TargetThread, handle);
	}
};

class AkSemaphore
{
public:
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
	AkThreadType m_TargetThread;

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