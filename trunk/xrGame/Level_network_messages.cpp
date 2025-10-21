#include "stdafx.h"
#include "level.h"
#include "xrmessages.h"
#include "net_queue.h"
#include "ai_space.h"
#include "saved_game_wrapper.h"
#include "level_graph.h"

void CLevel::ClientReceive()
{
	for (NET_Packet* P = net_msg_Retreive(); P; P=net_msg_Retreive())
	{
		u16			m_type;
		P->r_begin	(m_type);

		net_msg_Release();
	}	
}

void				CLevel::OnMessage				(void* data, u32 size)
{	
	IPureClient::OnMessage(data, size);	
};

NET_Packet*				CLevel::net_msg_Retreive		()
{
	return IPureClient::net_msg_Retreive();
}

