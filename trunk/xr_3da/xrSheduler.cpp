#include "stdafx.h"
#include "xrSheduler.h"

//#define DEBUG_SCHEDULER

//-------------------------------------------------------------------------------------
void CSheduler::Initialize		()
{
	m_processing_now	= false;
}

void CSheduler::Destroy			()
{
	internal_Registration		();

	for (u32 it=0; it<Items.size(); it++)
	{
		if (0==Items[it].Object)	
		{
			Items.erase(Items.begin()+it);
			it	--;
		}
	}
#ifdef DEBUG	
	if (!Items.empty())
	{
		Msg("! Sheduler work-list is not empty");
		for (u32 it = 0; it < Items.size(); it++)
			Msg("%s", Items[it].Object->shedule_Name().c_str());
	}
#endif // DEBUG
	Items.clear			();
	Registration.clear	();
}

#ifdef FRAME_SCHEDULER
#include "igame_level.h"

void CSheduler::AddSchedulerFrame()
{
	//if (psDeviceFlags.test(1ul << 23ul))
		Device.seqFrameMT.Add(this, REG_PRIORITY_HIGH);
	//else
	//	Device.seqFrame.Add(this, REG_PRIORITY_HIGH);
}

void CSheduler::RemoveSchedulerFrame()
{
	//if (psDeviceFlags.test(mtScheduler))
		Device.seqFrameMT.Remove(this);
	//else
	//	Device.seqFrame.Remove(this);
}

void CSheduler::OnFrame()
{
	if (Device.Paused() || !g_pGameLevel)
		return;

	UpdateScheduler();
}
#endif

void	CSheduler::internal_Registration()
{
	for (u32 it=0; it<Registration.size(); it++)
	{
		ItemReg&	R	= Registration	[it];
		if (R.OP)	{
			// register
			// search for paired "unregister"
			BOOL	bFoundAndErased		= FALSE;
			for (u32 pair=it+1; pair<Registration.size(); pair++)
			{
				ItemReg&	R_pair	= Registration	[pair];
				if	((!R_pair.OP)&&(R_pair.Object == R.Object))	{
					bFoundAndErased		= TRUE;
					Registration.erase	(Registration.begin()+pair	);
					break				;
				}
			}

			// register if non-paired
			if (!bFoundAndErased)		{
#ifdef DEBUG_SCHEDULER
				Msg						("SCHEDULER: internal register [%s][%x]", R.Object->shedule_Name().c_str(), R.Object);
#endif // DEBUG_SCHEDULER
				internal_Register		(R.Object);
			}
#ifdef DEBUG_SCHEDULER
			else 
				Msg						("SCHEDULER: internal register skipped, because unregister found [%s][%x]","unknown",R.Object);
#endif // DEBUG_SCHEDULER
		}
		else		{
			// unregister
			internal_Unregister			(R.Object);
		}
	}
	Registration.clear	();
}

void CSheduler::internal_Register	(ISheduled* O)
{
	Item						TNext;
	TNext.dwTimeForExecute = Device.dwTimeGlobal;
	TNext.dwTimeOfLastExecute = Device.dwTimeGlobal;
	TNext.Object = O;
	TNext.scheduled_name = O->shedule_Name();

	Push(TNext);
}

bool CSheduler::internal_Unregister	(ISheduled* O, bool warn_on_not_found)
{
	for (u32 i = 0; i < Items.size(); i++)
	{
		if (Items[i].Object == O) {
#ifdef DEBUG_SCHEDULER
			Msg("SCHEDULER: internal unregister [%s][%x][%s]", Items[i].scheduled_name.c_str(), O, "false");
#endif // DEBUG_SCHEDULER
			Items[i].Object = nullptr;
			return true;
		}
	}

#ifdef DEBUG
	if (warn_on_not_found)
		Msg							("! scheduled object %s tries to unregister but is not registered",O->shedule_Name().c_str());
#endif // DEBUG

	return false;
}

#ifdef DEBUG
bool CSheduler::Registered		(ISheduled *object) const
{
	u32							count = 0;
	typedef xr_vector<Item>		ITEMS;

	{
		ITEMS::const_iterator	I = Items.begin();
		ITEMS::const_iterator	E = Items.end();
		for ( ; I != E; ++I)
			if ((*I).Object == object) {
//				Msg				("0x%8x found in non-RT",object);
				VERIFY			(!count);
				count			= 1;
				break;
			}
	}

	typedef xr_vector<ItemReg>	ITEMS_REG;
	ITEMS_REG::const_iterator	I = Registration.begin();
	ITEMS_REG::const_iterator	E = Registration.end();
	for ( ; I != E; ++I) {
		if ((*I).Object == object) {
			if ((*I).OP) {
//				Msg				("0x%8x found in registration on register",object);
				VERIFY			(!count);
				++count;
			}
			else {
//				Msg				("0x%8x found in registration on UNregister",object);
				VERIFY			(count == 1);
				--count;
			}
		}
	}

	VERIFY						(!count || (count == 1));
	return						(count == 1);
}
#endif // DEBUG

void	CSheduler::Register		(ISheduled* A)
{
	VERIFY		(!Registered(A));

	ItemReg		R;
	R.OP		= TRUE				;
	R.Object	= A					;

#ifdef DEBUG_SCHEDULER
	Msg			("SCHEDULER: register [%s][%x]", A->shedule_Name().c_str(), A);
#endif // DEBUG_SCHEDULER

	Registration.push_back	(R);
}

void	CSheduler::Unregister	(ISheduled* A						)
{
	VERIFY		(Registered(A));

#ifdef DEBUG_SCHEDULER
	Msg			("SCHEDULER: unregister [%s][%x]",A->shedule_Name().c_str(), A);
#endif // DEBUG_SCHEDULER

	if (m_processing_now) {
		if (internal_Unregister(A, false))
			return;
	}

	ItemReg		R;
	R.OP		= FALSE				;
	R.Object	= A					;

	Registration.push_back			(R);
}

void CSheduler::Push				(Item& I)
{
	Items.push_back	(I);
	std::push_heap	(Items.begin(), Items.end());
}

void CSheduler::Pop					()
{
	std::pop_heap	(Items.begin(), Items.end());
	Items.pop_back	();
}

void CSheduler::ProcessStep			()
{
	// Normal priority
	u32		dwTime					= Device.dwTimeGlobal;
	CTimer							eTimer;
	for (u32 i = 0; i < Items.size(); ++i) {
		// Update
		Item& T = Items[i];

		if (T.dwTimeForExecute >= dwTime)
			continue;

#ifdef DEBUG_SCHEDULER
		Msg		("SCHEDULER: process step [%s][%x][false]", T.scheduled_name.c_str(), T.Object);
#endif // DEBUG_SCHEDULER
		u32		Elapsed				= dwTime-T.dwTimeOfLastExecute;

		if (!T.Object || !T.Object->shedule_Needed()) {
			// Erase element
			if (!T.Object)
			{
#ifdef DEBUG_SCHEDULER
				Msg("!!! [%s]: Unknown object for scheduler! Delete from scheduler objects! Scheduler name: [%s]!!!", __FUNCTION__, T.scheduled_name.c_str());
#endif	
			}
			
//			Unregister(T.Object);
			continue;
		}

#ifdef DEBUG
		T.Object->dbg_startframe = Device.dwFrame;
		eTimer.Start();
#endif // DEBUG

		u32 ScheduleDelta = 0;
		if (g_pGameLevel->CurrentEntity() == T.Object->dcast_CObject())
			ScheduleDelta = Elapsed;
		else
		{
			// Calc next update interval
			u32		dwMin = _max(u32(30), T.Object->shedule.t_min);
			u32		dwMax = (1000 + T.Object->shedule.t_max) / 2;
			float	scale = T.Object->shedule_Scale();
			u32		dwUpdate = dwMin + iFloor(float(dwMax - dwMin) * scale);
			clamp(dwUpdate, u32(_max(dwMin, u32(20))), dwMax);
			T.dwTimeForExecute = dwTime + dwUpdate;

			ScheduleDelta = clampr(Elapsed, u32(1), u32(_max(u32(T.Object->shedule.t_max), u32(1000))));
		}

		T.dwTimeOfLastExecute = dwTime;
		T.Object->shedule_Update(ScheduleDelta);
#ifdef DEBUG
		u32	execTime = eTimer.GetElapsed_ms();
#endif

#ifdef DEBUG
		if (execTime > 15) {
			Msg("* xrSheduler: too much time consumed by object [%s] (%dms)", T.scheduled_name.c_str(), execTime);
		}
#endif
	}
}

void CSheduler::UpdateScheduler()
{	
	// Initialize
	Device.Statistic->Sheduler.Begin();
	internal_Registration			();

#ifdef DEBUG_SCHEDULER
	Msg								("SCHEDULER: PROCESS STEP %d",Device.dwFrame);
#endif // DEBUG_SCHEDULER
	// Realtime priority
	m_processing_now				= true;
	// Normal (sheduled)
	ProcessStep						();
	m_processing_now				= false;
#ifdef DEBUG_SCHEDULER
	Msg								("SCHEDULER: PROCESS STEP FINISHED %d",Device.dwFrame);
#endif // DEBUG_SCHEDULER

	// Finalize
	internal_Registration			();
	Device.Statistic->Sheduler.End	();
}
