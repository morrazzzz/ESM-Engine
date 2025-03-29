#include "stdafx.h"
#include "net_server.h"

#pragma warning(push)
#pragma warning(disable:4995)
#include <malloc.h>
#pragma warning(pop)

XRNETSERVER_API ClientID BroadcastCID(0xffffffff);

void ip_address::set(LPCSTR src_string)
{
	u32		buff[4];
	int cnt = sscanf(src_string, "%d.%d.%d.%d", &buff[0], &buff[1], &buff[2], &buff[3]);
	if(cnt==4)
	{
		m_data.a1	= u8(buff[0]&0xff);
		m_data.a2	= u8(buff[1]&0xff);
		m_data.a3	= u8(buff[2]&0xff);
		m_data.a4	= u8(buff[3]&0xff);
	}else
	{
		Msg			("! Bad ipAddress format [%s]",src_string);
		m_data.data		= 0;
	}
}

xr_string ip_address::to_string() const
{
	string128	res;
	sprintf_s	(res,sizeof(res),"%d.%d.%d.%d", m_data.a1, m_data.a2, m_data.a3, m_data.a4);
	return		res;
}

IClient::IClient( CTimer* timer )
{
	dwTime_LastUpdate	= 0;
}

IClient::~IClient()
{
}

IClient*	IPureServer::ID_to_client		(ClientID ID, bool ScanAll)
{
	if ( 0 == ID.value() )			return NULL;
	csPlayers.Enter	();

	for ( u32 client = 0; client < net_Players.size(); ++client )
	{
		if ( net_Players[client]->ID == ID )
		{
			csPlayers.Leave();
			return net_Players[client];
		}
	}
	if ( ScanAll )
	{
		for ( u32 client = 0; client < net_Players_disconnected.size(); ++client )
		{
			if ( net_Players_disconnected[client]->ID == ID )
			{
				csPlayers.Leave();
				return net_Players_disconnected[client];
			}
		}
	};
	csPlayers.Leave();
	return NULL;
}

//==============================================================================

IPureServer::IPureServer	(CTimer* timer, BOOL	Dedicated)
	:	m_bDedicated(Dedicated)
#ifdef PROFILE_CRITICAL_SECTIONS
	,csPlayers(MUTEX_PROFILE_ID(IPureServer::csPlayers))
	,csMessage(MUTEX_PROFILE_ID(IPureServer::csMessage))
#endif // PROFILE_CRITICAL_SECTIONS
{
	device_timer			= timer;
	SV_Client				= NULL;
}

IPureServer::~IPureServer	()
{
	SV_Client					= NULL;
}

IPureServer::EConnect IPureServer::Connect(LPCSTR options)
{
	connect_options			= options;
	return	ErrNoError;
}

void IPureServer::Disconnect	()
{
}

void	IPureServer::SendTo		(ClientID ID/*DPNID ID*/, NET_Packet& P, u32 dwFlags, u32 dwTimeout)
{
	SendTo_LL( ID, P.B.data, P.B.count, dwFlags, dwTimeout );
}

void	IPureServer::SendBroadcast_LL(ClientID exclude, void* data, u32 size, u32 dwFlags)
{
	for( u32 i=0; i<net_Players.size(); i++ )
	{
		IClient* player = net_Players[i];
		
		if( player->ID == exclude )     continue;
		
		SendTo_LL( player->ID, data, size, dwFlags );
	}
}

void	IPureServer::SendBroadcast(ClientID exclude, NET_Packet& P, u32 dwFlags)
{
	// Perform broadcasting
	SendBroadcast_LL( exclude, P.B.data, P.B.count, dwFlags );
}

u32	IPureServer::OnMessage	(NET_Packet& P, ClientID sender)	// Non-Zero means broadcasting with "flags" as returned
{
	return 0;
}

void IPureServer::OnCL_Connected		(IClient* CL)
{
	Msg("* Player '%s' connected.\n",	CL->name.c_str());
}
void IPureServer::OnCL_Disconnected		(IClient* CL)
{
	Msg("* Player '%s' disconnected.\n",CL->name.c_str());
}

