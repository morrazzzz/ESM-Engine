#include "stdafx.h"
#include "alife_schedule_registry.h"

void CALifeScheduleRegistry::add(CSE_ALifeDynamicObject *object)
{
	if (CSE_ALifeSchedulable* schedulable = smart_cast<CSE_ALifeSchedulable*>(object))
	{
		if (!schedulable->need_update(object))
			return;

		inherited::add(object->ID, schedulable);
	}
}

void CALifeScheduleRegistry::remove	(CSE_ALifeDynamicObject *object, bool no_assert)
{
	if (CSE_ALifeSchedulable* schedulable = smart_cast<CSE_ALifeSchedulable*>(object))
		inherited::remove(object->ID, no_assert || !schedulable->need_update(object));
}

