#include "stdafx.h"
#include "xrServer.h"
#include "xrServer_Objects.h"

void xrServer::Process_save(NET_Packet& P)
{
	// while has information
	while (!P.r_eof())
	{
		// find entity
		u16				ID;
		u16				size;

		P.r_u16			(ID);
		P.r_u16			(size);
		s32				_pos_start	= P.r_tell	();
		CSE_Abstract	*E	= ID_to_entity(ID);

		if (E) {
			E->net_Ready = TRUE;
			E->load		(P);
		}
		else
			P.r_advance	(size);
		s32				_pos_end	= P.r_tell	();
		s32				_size		= size;
		if				(_size != (_pos_end-_pos_start))	{
			Msg			("! load/save mismatch, object: '%s'",E?E->name_replace():"unknown");
			s32			_rollback	= _pos_start+_size;
			P.r_seek	(_rollback);
		}
	}
}
