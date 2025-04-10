#pragma once

#include "net_shared.h"

class XRNETSERVER_API INetQueue
{
	xrCriticalSection		cs;
	xr_deque<NET_Packet*>	ready;
	xr_vector<NET_Packet*>	unused;
public:
	INetQueue();
	~INetQueue();

	NET_Packet*			CreateGet	();
//.	NET_Packet*			CreateGet	(const NET_Packet& _other);
	void				CreateCommit(NET_Packet*);

	NET_Packet*			Retreive();
	void				Release	();
};

//==============================================================================

class XRNETSERVER_API IPureClient
{
protected:
	CTimer*					device_timer;

	INetQueue				net_Queue;

	s32						net_TimeDelta;
public:
	IPureClient				(CTimer* tm);
	virtual ~IPureClient	();
	
	bool Connect();

	// receive
	IC virtual	NET_Packet*			net_msg_Retreive		()	{ return net_Queue.Retreive();	}
	IC void					net_msg_Release			()	{ net_Queue.Release();			}

	// send
	virtual void			OnMessage				(void* data, u32 size);
	
	// time management
	IC u32					timeServer				()	{ return TimeGlobal(device_timer) + net_TimeDelta; }
	IC u32					timeServer_Async		()	{ return TimerAsync(device_timer) + net_TimeDelta; }
	IC u32					timeServer_Delta		()	{ return net_TimeDelta; }
};

