#include "stdafx.h"
#include "actor.h"
#include "customdetector.h"
#include "uigamesp.h"
#include "hudmanager.h"
#include "artifact.h"
#include "inventory.h"
#include "level.h"
#include "xr_level_controller.h"
#include "FoodItem.h"
#include "ActorCondition.h"

#include "CameraFirstEye.h"
#include "holder_custom.h"
#include "ui/uiinventoryWnd.h"
#ifdef DEBUG
#include "PHDebug.h"
#endif

void CActor::OnEvent		(NET_Packet& P, u16 type)
{
	inherited::OnEvent			(P,type);
	CInventoryOwner::OnEvent	(P,type);

	u16 id;
	switch (type)
	{
	case GEG_PLAYER_ACTIVATEARTEFACT:
		{
			P.r_u16		(id);
			CObject* O	= Level().Objects.net_Find	(id);
			if(!O)		break;
			if (O->getDestroy()) 
			{
#ifdef DEBUG
				Msg("! something to destroyed object - %s[%d]0x%X", *O->cName(), id, smart_cast<CInventoryItem*>(O));
#endif
				break;
			}
			switch (type)
			{
			case GEG_PLAYER_ACTIVATEARTEFACT:
				{
					CArtefact* pArtefact		= smart_cast<CArtefact*>(O);
					pArtefact->ActivateArtefact	();
				}break;
			}
		}break;
	case GEG_PLAYER_WEAPON_HIDE_STATE:
		{
			u32 State		= P.r_u32();
			BOOL	Set		= !!P.r_u8();
			inventory().SetSlotsBlocked	((u16)State, !!Set);
		}break;
	case GEG_PLAYER_ATTACH_HOLDER:
		{
			u32 id = P.r_u32();
			CObject* O	= Level().Objects.net_Find	(id);
			if (!O){
				Msg("! Error: No object to attach holder [%d]", id);
				break;
			}
			VERIFY(m_holder==NULL);
			CHolderCustom*	holder = smart_cast<CHolderCustom*>(O);
			if(!holder->Engaged())	use_Holder		(holder);

		}break;
	case GEG_PLAYER_DETACH_HOLDER:
		{
			if			(!m_holder)	break;
			u32 id			= P.r_u32();
			CGameObject*	GO	= smart_cast<CGameObject*>(m_holder);
			VERIFY			(id==GO->ID());
			use_Holder		(NULL);
		}break;
	}
}

void			CActor::MoveActor		(Fvector NewPos, Fvector NewDir)
{
	Fmatrix	M = XFORM();
	M.translate(NewPos);
	r_model_yaw				= NewDir.y;
	r_torso.yaw				= NewDir.y;
	r_torso.pitch			= -NewDir.x;
	unaffected_r_torso.yaw	= r_torso.yaw;
	unaffected_r_torso.pitch= r_torso.pitch;
	unaffected_r_torso.roll	= 0;//r_torso.roll;

	r_torso_tgt_roll		= 0;
	cam_Active()->Set		(-unaffected_r_torso.yaw,unaffected_r_torso.pitch,unaffected_r_torso.roll);
	ForceTransform(M);

	m_bInInterpolation = false;	
}