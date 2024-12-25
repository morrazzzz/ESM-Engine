#pragma once

#include <mutex>

// Desc: Simpl wrapper for critical section
class xrCriticalSection final : std::recursive_mutex
{
	using inherited = std::recursive_mutex;
public:
	inline void Enter() { 
		PROF_EVENT("xrCriticalSection::Lock");
		inherited::lock(); }
	inline void Leave() 
	{
		PROF_EVENT("xrCriticalSection::Unlock");
		inherited::unlock(); 
	}
	inline bool TryEnter() { return inherited::try_lock(); }
};
