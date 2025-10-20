#pragma once

#include "xrMessages.h"

extern BOOL		g_bCheckTime;

class	NET_Event
{
public:
	u16					ID;
	u32					timestamp;
	u16					type;
	u16					destination;
	xr_vector<u8>		data;
public:
	void				import		(NET_Packet& P)
	{
		data.clear		();
		P.r_begin		(ID			);	//VERIFY(M_EVENT==ID);

		u32 size		= P.r_elapsed();
		if (size)	
		{
			data.resize		(size);
			P.r				(&*data.begin(),size);
		}
	}
};

IC bool operator < (const NET_Event& A, const NET_Event& B)	{ return A.timestamp<B.timestamp; }



class	NET_Queue_Event
{
public:
//	xr_multiset<NET_Event>	queue;	
	xr_deque<NET_Event>	queue;
public:
	IC void				insert		(NET_Packet& P)
	{
		NET_Event		E;
		E.import		(P);
		queue.push_back	(E);
	}

	IC bool	available()
	{
		if (queue.empty()) 
			return false;

		return true;
	}

	IC void				get			(u16& ID, u16& dest, u16& type, NET_Packet& P)
	{
	}
};
