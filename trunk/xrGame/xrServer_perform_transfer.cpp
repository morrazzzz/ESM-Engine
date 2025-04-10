#include "stdafx.h"
#include "xrserver.h"
#include "xrmessages.h"
#include "xrserver_objects.h"

void xrServer::Perform_reject(CSE_Abstract* what, CSE_Abstract* from, int delta)
{
	R_ASSERT				(what && from);
	R_ASSERT				(what->ID_Parent == from->ID);

	NET_Packet				P;
	u32						time = Device.dwTimeGlobal - delta;

	P.w_begin				(M_EVENT);
	P.w_u32					(time);
	P.w_u16					(GE_OWNERSHIP_REJECT);
	P.w_u16					(from->ID);
	P.w_u16					(what->ID);
	P.w_u8					(1);

	Process_event_reject	(P,time,from->ID,what->ID);
}
