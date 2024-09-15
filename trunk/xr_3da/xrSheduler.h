#pragma once

#include "ISheduled.h"

//#define DEBUG_SCHEDULER //enable debugging scheduler.

#define FRAME_SCHEDULER

#ifdef FRAME_SCHEDULER
#include "device.h"

class ENGINE_API CSheduler : public pureFrame
#else
class ENGINE_API CSheduler
#endif
{
	struct Item
	{
		u32			dwTimeForExecute;
		u32			dwTimeOfLastExecute;
		shared_str	scheduled_name;
		ISheduled* Object;

		IC bool		operator < (Item& I)
		{
			return dwTimeForExecute > I.dwTimeForExecute;
		}
	};
	struct	ItemReg
	{
		BOOL		OP;
		BOOL		RT;
		ISheduled* Object;
	};

	xr_vector<Item>			ItemsRT;
	xr_vector<Item>			Items;
	xr_vector<Item>			ItemsProcessed;
	xr_vector<ItemReg>		Registration;

	u64	cycles_start;
	u64	cycles_limit;

	bool m_processing_now;
public:
#ifdef FRAME_SCHEDULER
	void OnFrame() override;

	void AddSchedulerFrame();
	void RemoveSchedulerFrame();
#else
	void ProcessStep();
	void UpdateScheduler();
#endif

#ifdef DEBUG
	bool			Registered(ISheduled* object) const;
#endif // DEBUG
	void			Register(ISheduled* A, BOOL RT = FALSE);
	void			Unregister(ISheduled* A);

	void			Initialize();
	void			Destroy();
private:


	IC void			Push(Item& I);
	IC void			Pop();
	IC Item& Top()
	{
		return Items.front();
	}
	void			internal_Register(ISheduled* A, BOOL RT = FALSE);
	bool			internal_Unregister(ISheduled* A, BOOL RT, bool warn_on_not_found = true);
	void			internal_Registration();

#ifdef FRAME_SCHEDULER
	void ProcessStep();
	void UpdateScheduler();
#endif
};
