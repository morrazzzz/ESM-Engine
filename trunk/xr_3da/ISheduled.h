#pragma once

class	ENGINE_API	ISheduled
{
public:
	struct {
		u32		t_min		:	14;		// minimal bound of update time (sample: 20ms)
		u32		t_max		:	14;		// maximal bound of update time (sample: 200ms)
		u32		b_RT		:	1;
	}	shedule;

#ifdef DEBUG
	u32									dbg_startframe;
	u32									dbg_update_shedule;
#endif

				ISheduled				();
	virtual ~	ISheduled				();

	void								shedule_register	();
	void								shedule_unregister	();

	virtual float						shedule_Scale		()			= 0;
	virtual void						shedule_Update		(u32 dt);
	virtual	shared_str					shedule_Name		() const	{ return shared_str("unknown"); };
	virtual bool						shedule_Needed		()			= 0;

};
