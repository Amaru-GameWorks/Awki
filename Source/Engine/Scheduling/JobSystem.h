#pragma once
#include "Core/Log.h"
#include "Core/Assert.h"
#include "Utilities/AkId.h"

#include <mutex>
#include <vector>
#include <thread>
#include <coroutine>
#include <unordered_map>
#include <condition_variable>

class AkJobSystem;

struct AkJob
{
	struct promise_type;
	using handle_type = std::coroutine_handle<promise_type>;

	struct promise_type
	{
		AkJob get_return_object() { return AkJob{ handle_type::from_promise(*this) }; }
		std::suspend_never initial_suspend() noexcept { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		void return_void() noexcept {}
		void unhandled_exception()
		{
			std::exception_ptr exceptionPointer = std::current_exception();
			try
			{
				if (exceptionPointer)
				{
					std::rethrow_exception(exceptionPointer);
				}
			}
			catch (const std::exception& exception)
			{
				AkLogError("Coroutine exception: {}", exception.what());
			}
		}
	};

	handle_type handle;
};

struct AkJobInitialSuspend
{
	struct promise_type;
	using handle_type = std::coroutine_handle<promise_type>;

	struct promise_type
	{
		AkJobInitialSuspend get_return_object() { return AkJobInitialSuspend{ handle_type::from_promise(*this) }; }
		std::suspend_always initial_suspend() noexcept { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		void return_void() noexcept {}
		void unhandled_exception()
		{
			std::exception_ptr exceptionPointer = std::current_exception();
			try
			{
				if (exceptionPointer)
				{
					std::rethrow_exception(exceptionPointer);
				}
			}
			catch (const std::exception& exception)
			{
				AkLogError("Coroutine exception: {}", exception.what());
			}
		}
	};

	handle_type handle;
};

enum class AkThreadType
{
	Worker,
	Render,
	Main
};

class AkJobSystem
{
public:
	static void Initialize(size_t numThreads = std::max(std::thread::hardware_concurrency() - 2, 1u))
	{
		for (size_t i = 0; i < numThreads; ++i)
		{
			m_Workers.emplace_back([]
			{
				WorkerLoop();
			});
		}

		m_RenderThread = std::thread(RenderThreadLoop);
	}

	static void DeInitialize()
	{
		m_Stop.store(true, std::memory_order_relaxed);

		m_MainConditional.notify_all();
		m_WorkerConditional.notify_all();
		m_RenderConditional.notify_all();

		for (auto& worker : m_Workers)
			if (worker.joinable())
				worker.join();
		m_Workers.clear();

		if (m_RenderThread.joinable())
			m_RenderThread.join();
	}

	static void Schedule(AkThreadType targetThread, AkJob job)
	{
		Schedule(targetThread, job.handle);
	}

	static void Schedule(AkThreadType targetThread, AkJobInitialSuspend job)
	{
		Schedule(targetThread, job.handle);
	}

	static void Schedule(AkThreadType targetThread, std::coroutine_handle<> handle)
	{
		if (!handle)
			return;

		switch (targetThread)
		{
			case AkThreadType::Worker:
			{
				std::lock_guard<std::mutex> lock(m_WorkerJobQueueMutex);
				m_WorkerJobQueue.push_back(handle);
				m_WorkerConditional.notify_one();
				break;
			}
			case AkThreadType::Main:
			{
				std::lock_guard<std::mutex> lock(m_MainJobQueueMutex);
				m_MainJobQueue.push_back(handle);
				m_MainConditional.notify_one();
				break;
			}
			case AkThreadType::Render:
			{
				std::lock_guard<std::mutex> lock(m_RenderJobQueueMutex);
				m_RenderJobQueue.push_back(handle);
				m_RenderConditional.notify_one();
				break;
			}
		}
	}

	static void ProcessMainThreadJobOrWait()
	{
		std::coroutine_handle<> job;
		{
			std::unique_lock<std::mutex> lock(m_MainJobQueueMutex);
			m_MainConditional.wait(lock, []
			{
				return m_Stop.load(std::memory_order_relaxed) || !m_MainJobQueue.empty();
			});

			if (m_MainJobQueue.empty())
				return;

			job = m_MainJobQueue.back();
			m_MainJobQueue.pop_back();
		}

		if (job)
			job.resume();
	}

private:
	static void WorkerLoop()
	{
		while (!m_Stop.load(std::memory_order_relaxed))
		{
			std::coroutine_handle<> job;
			{
				std::unique_lock<std::mutex> lock(m_WorkerJobQueueMutex);
				m_WorkerConditional.wait(lock, []
				{
					return m_Stop.load(std::memory_order_relaxed) || !m_WorkerJobQueue.empty();
				});

				if (m_WorkerJobQueue.empty())
					return;

				job = m_WorkerJobQueue.back();
				m_WorkerJobQueue.pop_back();
			}

			if (job)
				job.resume();
		}
	}

	static void RenderThreadLoop()
	{
		while (!m_Stop.load(std::memory_order_relaxed))
		{
			std::coroutine_handle<> job;
			{
				std::unique_lock<std::mutex> lock(m_RenderJobQueueMutex);
				m_RenderConditional.wait(lock, []
				{
					return m_Stop.load(std::memory_order_relaxed) || !m_RenderJobQueue.empty();
				});

				if (m_RenderJobQueue.empty())
					return;

				job = m_RenderJobQueue.back();
				m_RenderJobQueue.pop_back();
			}

			if (job)
				job.resume();
		}
	}

	static inline std::thread m_RenderThread;
	static inline std::vector<std::thread> m_Workers;

	static inline std::mutex m_MainJobQueueMutex;
	static inline std::mutex m_WorkerJobQueueMutex;
	static inline std::mutex m_RenderJobQueueMutex;

	static inline std::condition_variable m_MainConditional;
	static inline std::condition_variable m_WorkerConditional;
	static inline std::condition_variable m_RenderConditional;

	static inline std::vector<std::coroutine_handle<>> m_MainJobQueue;
	static inline std::vector<std::coroutine_handle<>> m_WorkerJobQueue;
	static inline std::vector<std::coroutine_handle<>> m_RenderJobQueue;

	static inline std::atomic_bool m_Stop = false;
};