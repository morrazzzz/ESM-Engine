#include "stdafx.h"
#include "xrSheduler.h"
#include "xr_object.h"

#ifndef DEBUG
#pragma todo("Why would he delete something that is not registered? why 'sad'???")
#define UNREGISTER_IN_DESTRUCTOR
#endif

ISheduled::ISheduled	()	
{
	shedule.t_min		= 20;
	shedule.t_max		= 1000;
#ifdef DEBUG
	dbg_startframe		= 1;
	dbg_update_shedule	= 0;
#endif
}

ISheduled::~ISheduled	()
{
	VERIFY2				(
		!Engine.Sheduler.Registered(this),
		make_string("0x%08x : %s",this,*shedule_Name())
	);

	// Strange code: Begin. ????
	// sad, but true
	// we need this to become MASTER_GOLD
#ifdef UNREGISTER_IN_DESTRUCTOR
	Engine.Sheduler.Unregister				(this);
#endif // DEBUG
	// Strange code: End. ????
}

void	ISheduled::shedule_register			()
{
	Engine.Sheduler.Register				(this);
}

void	ISheduled::shedule_unregister		()
{
	Engine.Sheduler.Unregister				(this);
}

void	ISheduled::shedule_Update			(u32 dt)
{
#ifdef DEBUG
	if (dbg_startframe==dbg_update_shedule)	
	{
		LPCSTR		name	= "unknown";
		CObject*	O		= dynamic_cast<CObject*>	(this);
		if			(O)		name	= *O->cName();
		Debug.fatal	(DEBUG_INFO,"'shedule_Update' called twice per frame for %s",name);
	}
	dbg_update_shedule	= dbg_startframe;
#endif
}
