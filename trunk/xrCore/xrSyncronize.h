#pragma once

#include <mutex>

// Desc: Simpl wrapper for critical section
class xrCriticalSection final : std::recursive_mutex
{
public:
	inline void Enter() { __super::lock(); }
	inline void Leave() { __super::unlock(); }
	inline bool TryEnter() { return __super::try_lock(); }
};
