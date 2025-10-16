#include "stdafx.h"
#include "net_client.h"
#include "net_server.h"

INetQueue::INetQueue()		
#ifdef PROFILE_CRITICAL_SECTIONS
	:cs(MUTEX_PROFILE_ID(INetQueue))
#endif // PROFILE_CRITICAL_SECTIONS
{
	unused.reserve	(128);
	for (int i=0; i<16; i++)
		unused.push_back	(xr_new<NET_Packet>());
}

INetQueue::~INetQueue()
{
	cs.Enter		();
	u32				it;
	for				(it=0; it<unused.size(); it++)	xr_delete(unused[it]);
	for				(it=0; it<ready.size(); it++)	xr_delete(ready[it]);
	cs.Leave		();
}

static u32 LastTimeCreate = 0;

void INetQueue::CreateCommit(NET_Packet* P)
{
	cs.Enter		();
	ready.push_back	(P);
	cs.Leave		();
}

NET_Packet*		INetQueue::CreateGet()
{
	NET_Packet*	P			= 0;
	cs.Enter		();

	if (unused.empty())	
	{
		P					= xr_new<NET_Packet> ();

//.		ready.push_back		(xr_new<NET_Packet> ());
//.		P					= ready.back	();

		LastTimeCreate = GetTickCount();
	} else 
	{
		P					= unused.back();
//.		ready.push_back		(unused.back());
		unused.pop_back		();
//.		P					= ready.back	();
	}
	cs.Leave		();
	return	P;
}

NET_Packet*		INetQueue::Retreive	()
{
	NET_Packet*	P			= 0;
	cs.Enter		();
//#ifdef _DEBUG
//			Msg ("INetQueue::Retreive - ready %d, unused %d", ready.size(), unused.size());
//#endif
	if (!ready.empty())		P = ready.front();
	//---------------------------------------------	
	else
	{
		u32 tmp_time = GetTickCount()-60000;
		u32 size = unused.size();
		if ((LastTimeCreate < tmp_time) &&  (size > 32))
		{
			xr_delete(unused.back());
			unused.pop_back();
		}		
	}
	//---------------------------------------------	
	cs.Leave		();
	return	P;
}
void			INetQueue::Release	()
{
	cs.Enter		();
//#ifdef _DEBUG
//		Msg ("INetQueue::Release - ready %d, unused %d", ready.size(), unused.size());
//#endif
	VERIFY			(!ready.empty());
	//---------------------------------------------
	u32 tmp_time = GetTickCount()-60000;
	u32 size = unused.size();
	if ((LastTimeCreate < tmp_time) &&  (size > 32))
	{
		xr_delete(ready.front());
	}
	else
		unused.push_back(ready.front());
	//---------------------------------------------	
	ready.pop_front	();
	cs.Leave		();
}

//-------
//------------------------------------------------------------------------------

//==============================================================================

IPureClient::IPureClient(CTimer* timer)
{
	device_timer			= timer;
	net_TimeDelta			= 0;
}

IPureClient::~IPureClient	()
{
}

void	IPureClient::OnMessage(void* data, u32 size)
{
	// One of the messages - decompress it
	NET_Packet* P = net_Queue.CreateGet();

	P->construct			(data, size);
	P->timeReceive			= timeServer_Async();

	u16						tmp_type;
	P->r_begin				(tmp_type);
	net_Queue.CreateCommit	(P);
}
