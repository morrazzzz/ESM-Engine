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