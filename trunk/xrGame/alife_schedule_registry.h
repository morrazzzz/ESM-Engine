////////////////////////////////////////////////////////////////////////////
//	Module 		: alife_schedule_registry.h
//	Created 	: 15.01.2003
//  Modified 	: 12.05.2004
//	Author		: Dmitriy Iassenev
//	Description : ALife schedule registry
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "safe_map_iterator.h"
#include "xrServer_Objects_ALife.h"
#include "ai_debug.h"
#include "profiler.h"

class CALifeScheduleRegistry : public CSafeMapIterator<ALife::_OBJECT_ID,CSE_ALifeSchedulable,std::less<ALife::_OBJECT_ID>> {
private:
	struct CUpdatePredicate {
		u32								m_count;
		mutable u32						m_current;

		IC			CUpdatePredicate	(const u32 &count)
		{
			m_count						= count;
			m_current					= 0;
		}

		IC	bool	operator()			(CSE_ALifeSchedulable* i, u64 cycle_count) const
		{
			if (i->m_schedule_counter == cycle_count)
				return false;

			if (m_current >= m_count)
				return false;

			++m_current;
			i->m_schedule_counter = cycle_count;

			START_PROFILE("ALife/scheduled/update")
			i->update();
			STOP_PROFILE

			return true;
		}
	};

protected:
	typedef CSafeMapIterator<ALife::_OBJECT_ID,CSE_ALifeSchedulable,std::less<ALife::_OBJECT_ID>> inherited;

protected:
	u32		m_objects_per_update;

public:
	IC								CALifeScheduleRegistry	();
	virtual							~CALifeScheduleRegistry() = default;
			void					add						(CSE_ALifeDynamicObject *object);
			void					remove					(CSE_ALifeDynamicObject *object, bool no_assert = false);
	IC		void					update					();
	IC		CSE_ALifeSchedulable	*object					(const ALife::_OBJECT_ID &id, bool no_assert = false) const;
	IC		const u32				&objects_per_update		() const;
	IC		void					objects_per_update		(const u32 &objects_per_update);
};

#include "alife_schedule_registry_inline.h"