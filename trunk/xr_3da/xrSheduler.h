#pragma once

#include "ISheduled.h"

//#define DEBUG_SCHEDULER //enable debugging scheduler.

#define FRAME_SCHEDULER

#ifdef FRAME_SCHEDULER
#include "pure.h"

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
		bool OP;
		ISheduled* Object;
	};

	xr_vector<Item>			Items;
	xr_vector<ItemReg>		Registration;

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
	void			Register(ISheduled* A);
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
	void			internal_Register(ISheduled* A);
	bool			internal_Unregister(ISheduled* A, bool warn_on_not_found = true);
	void			internal_Registration();

#ifdef FRAME_SCHEDULER
	void ProcessStep();
	void UpdateScheduler();
#endif
};
