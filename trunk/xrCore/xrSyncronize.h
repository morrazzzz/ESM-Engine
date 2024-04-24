#pragma once

#include <mutex>

// Desc: Simpl wrapper for critical section
class xrCriticalSection final : std::recursive_mutex
{
	using inherited = std::recursive_mutex;
public:
	inline void Enter() { inherited::lock(); }
	inline void Leave() { inherited::unlock(); }
	inline bool TryEnter() { return inherited::try_lock(); }
};
