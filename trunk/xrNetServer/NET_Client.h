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
	enum ConnectionState
	{
		EnmConnectionFails=0,
		EnmConnectionWait=-1,
		EnmConnectionCompleted=1
	};
protected:
	struct HOST_NODE
	{
		DPN_APPLICATION_DESC	dpAppDesc;
		IDirectPlay8Address*	pHostAddress;
		shared_str				dpSessionName;
	};
	CTimer*					device_timer;
protected:
	xr_vector<HOST_NODE>	net_Hosts;

	ConnectionState			net_Connected;
	BOOL					net_Syncronised;

	INetQueue				net_Queue;

	s32						net_TimeDelta;
public:
	IPureClient				(CTimer* tm);
	virtual ~IPureClient	();
	
	bool Connect();
	void					Disconnect				();

	LPCSTR					net_SessionName			()	{ return *(net_Hosts.front().dpSessionName); }

	// receive
	IC virtual	NET_Packet*			net_msg_Retreive		()	{ return net_Queue.Retreive();	}
	IC void					net_msg_Release			()	{ net_Queue.Release();			}

	// send
	virtual void			OnMessage				(void* data, u32 size);
	
	// time management
	IC u32					timeServer				()	{ return TimeGlobal(device_timer) + net_TimeDelta; }
	IC u32					timeServer_Async		()	{ return TimerAsync(device_timer) + net_TimeDelta; }
	IC u32					timeServer_Delta		()	{ return net_TimeDelta; }

	virtual	BOOL			net_IsSyncronised		();
};

