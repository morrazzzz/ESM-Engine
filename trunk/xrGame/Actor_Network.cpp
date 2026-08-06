#include "stdafx.h"
#include "actor.h"
#include "Actor_Flags.h"
#include "inventory.h"
#include "xrserver_objects_alife_monsters.h"

#include "CameraFirstEye.h"

#include "ActorEffector.h"

#include "../xrPhysics/IPHWorld.h"
#include "level.h"
#include "alife_registry_wrappers.h"
#include "..\include\xrRender\Kinematics.h"
#include "client_spawn_manager.h"
#include "CharacterPhysicsSupport.h"
#include "Grenade.h"
#include "WeaponMagazined.h"
#include "CustomOutfit.h"

#include "actor_anim_defs.h"

#include "map_manager.h"
#include "UIGameCustom.h"
#include "ui/UIArtefactPanel.h"
#include "ui/UIMainIngameWnd.h"
#include "ui/UIHudStatesWnd.h"
#include "gamepersistent.h"
#include "game_object_space.h"
#include "GameTaskManager.h"
#include "holder_custom.h"
#include "actor_memory.h"
#include "actor_statistic_mgr.h"
#include "clsid_game.h"
#include "../xr_3da/xr_collide_form.h"
#ifdef DEBUG
#include "level_debug.h"
#include "debug_renderer.h"
#endif

int			g_cl_InterpolationType		= 0;
u32			g_cl_InterpolationMaxPoints = 0;
int			g_dwInputUpdateDelta		= 20;
BOOL		net_cl_inputguaranteed		= FALSE;
CActor*		g_actor						= NULL;

CActor*			Actor()	
{	
	VERIFY		(g_actor); 
	if (GameID() != GAME_SINGLE) 
		VERIFY	(g_actor == Level().CurrentControlEntity());
	return		(g_actor); 
};

//--------------------------------------------------------------------
void	CActor::ConvState(u32 mstate_rl, string128 *buf)
{
	strcpy(*buf,"");
	if (isActorAccelerated(mstate_rl, IsZoomAimingMode()))		strcat(*buf,"Accel ");
	if (mstate_rl&mcCrouch)		strcat(*buf,"Crouch ");
	if (mstate_rl&mcFwd)		strcat(*buf,"Fwd ");
	if (mstate_rl&mcBack)		strcat(*buf,"Back ");
	if (mstate_rl&mcLStrafe)	strcat(*buf,"LStrafe ");
	if (mstate_rl&mcRStrafe)	strcat(*buf,"RStrafe ");
	if (mstate_rl&mcJump)		strcat(*buf,"Jump ");
	if (mstate_rl&mcFall)		strcat(*buf,"Fall ");
	if (mstate_rl&mcTurn)		strcat(*buf,"Turn ");
	if (mstate_rl&mcLanding)	strcat(*buf,"Landing ");
	if (mstate_rl&mcLLookout)	strcat(*buf,"LLookout ");
	if (mstate_rl&mcRLookout)	strcat(*buf,"RLookout ");
	if (m_bJumpKeyPressed)		strcat(*buf,"+Jumping ");
};
//--------------------------------------------------------------------
void CActor::SaveCSEObj(CSE_Abstract* data, bool needSaveAll)
{
	CSE_ALifeCreatureActor* this_object = smart_cast<CSE_ALifeCreatureActor*>(data);

	this_object->fHealth = GetfHealth();
	this_object->timestamp = Device.dwTimeGlobal;
	this_object->o_Position = Position();
	this_object->o_model = angle_normalize(r_model_yaw);
	this_object->o_torso.yaw = angle_normalize(unaffected_r_torso.yaw);
	this_object->o_torso.pitch = angle_normalize(unaffected_r_torso.pitch);
	this_object->o_torso.roll = angle_normalize(unaffected_r_torso.roll);
	this_object->s_team = g_Team();
	this_object->s_squad = g_Squad();
	this_object->s_group = g_Group();

	u16 ms = static_cast<u16>(mstate_real & 0x0000ffff);
	this_object->mstate = ms;
	this_object->accel = NET_SavedAccel;
	this_object->velocity = character_physics_support()->movement()->GetVelocity();
	this_object->fRadiation = g_Radiation();
	this_object->weapon = u8(inventory().GetActiveSlot());

	if (!needSaveAll || !net_SaveRelevant())
		return;

	inherited::SaveCSEObj(data, needSaveAll);

	m_pPhysics_support->SaveStateSkeleton(data);
	this_object->m_holderID = m_holderID;
};

static void w_vec_q8(NET_Packet& P,const Fvector& vec,const Fvector& min,const Fvector& max)
{
	P.w_float_q8(vec.x,min.x,max.x);
	P.w_float_q8(vec.y,min.y,max.y);
	P.w_float_q8(vec.z,min.z,max.z);
}
static void r_vec_q8(NET_Packet& P,Fvector& vec,const Fvector& min,const Fvector& max)
{
	P.r_float_q8(vec.x,min.x,max.x);
	P.r_float_q8(vec.y,min.y,max.y);
	P.r_float_q8(vec.z,min.z,max.z);

	clamp(vec.x,min.x,max.x);
	clamp(vec.y,min.y,max.y);
	clamp(vec.z,min.z,max.z);
}
static void w_qt_q8(NET_Packet& P,const Fquaternion& q)
{
	//Fvector Q;
	//Q.set(q.x,q.y,q.z);
	//if(q.w<0.f)	Q.invert();
	//P.w_float_q8(Q.x,-1.f,1.f);
	//P.w_float_q8(Q.y,-1.f,1.f);
	//P.w_float_q8(Q.z,-1.f,1.f);
	///////////////////////////////////////////////////
	P.w_float_q8(q.x,-1.f,1.f);
	P.w_float_q8(q.y,-1.f,1.f);
	P.w_float_q8(q.z,-1.f,1.f);
	P.w_float_q8(q.w,-1.f,1.f);

	///////////////////////////////////////////


	//P.w_float_q8(q.x,-1.f,1.f);
	//P.w_float_q8(q.y,-1.f,1.f);
	//P.w_float_q8(q.z,-1.f,1.f);
	//P.w(sign())
}
static void r_qt_q8(NET_Packet& P,Fquaternion& q)
{
	//// x^2 + y^2 + z^2 + w^2 = 1
	//P.r_float_q8(q.x,-1.f,1.f);
	//P.r_float_q8(q.y,-1.f,1.f);
	//P.r_float_q8(q.z,-1.f,1.f);
	//float w2=1.f-q.x*q.x-q.y*q.y-q.z*q.z;
	//w2=w2<0.f ? 0.f : w2;
	//q.w=_sqrt(w2);
	/////////////////////////////////////////////////////
	///////////////////////////////////////////////////
	P.r_float_q8(q.x,-1.f,1.f);
	P.r_float_q8(q.y,-1.f,1.f);
	P.r_float_q8(q.z,-1.f,1.f);
	P.r_float_q8(q.w,-1.f,1.f);

	clamp(q.x,-1.f,1.f);
	clamp(q.y,-1.f,1.f);
	clamp(q.z,-1.f,1.f);
	clamp(q.w,-1.f,1.f);
}

#define F_MAX         3.402823466e+38F

static void	UpdateLimits (Fvector &p, Fvector& min, Fvector& max)
{
	if(p.x<min.x)min.x=p.x;
	if(p.y<min.y)min.y=p.y;
	if(p.z<min.z)min.z=p.z;

	if(p.x>max.x)max.x=p.x;
	if(p.y>max.y)max.y=p.y;
	if(p.z>max.z)max.z=p.z;

	for (int k=0; k<3; k++)
	{
		if (p[k]<min[k] || p[k]>max[k])
		{
			R_ASSERT2(0, "Fuck");
			UpdateLimits(p, min, max);
		}
	}
};

BOOL CActor::net_Spawn		(CSE_Abstract* DC)
{
	m_holder_id				= ALife::_OBJECT_ID(-1);
	m_feel_touch_characters = 0;
	m_snd_noise			= 0.0f;
	m_sndShockEffector	= NULL;
/*	m_followers			= NULL;*/
	if (m_pPhysicsShell)
	{
		m_pPhysicsShell->Deactivate();
		xr_delete(m_pPhysicsShell);
	};
	//force actor to be local on server client
	CSE_Abstract			*e	= (CSE_Abstract*)(DC);
	CSE_ALifeCreatureActor	*E	= smart_cast<CSE_ALifeCreatureActor*>(e);	
	
	if(TRUE == E->s_flags.is(M_SPAWN_OBJECT_ASPLAYER))
		g_actor = this;

	VERIFY(m_pActorEffector == NULL);

	m_pActorEffector			= xr_new<CActorCameraManager>();

	// motions
	m_bAnimTorsoPlayed			= false;
	m_current_legs_blend		= 0;
	m_current_jump_blend		= 0;
	m_current_legs.invalidate	();
	m_current_torso.invalidate	();
	m_current_head.invalidate	();
	//-------------------------------------
	// инициализаци€ реестров, используемых актером
	encyclopedia_registry->registry().init(ID());
	game_news_registry->registry().init(ID());


	if (!CInventoryOwner::net_Spawn(DC)) return FALSE;
	if (!inherited::net_Spawn(DC))	return FALSE;

	CSE_ALifeTraderAbstract	 *pTA	= smart_cast<CSE_ALifeTraderAbstract*>(e);
	set_money				(pTA->m_dwMoney, false);

	//убрать все артефакты с по€са
	m_ArtefactsOnBelt.clear();
//.	if(	TRUE == E->s_flags.test(M_SPAWN_OBJECT_LOCAL) && TRUE == E->s_flags.is(M_SPAWN_OBJECT_ASPLAYER))
//.		HUD().GetUI()->UIMainIngameWnd->m_artefactPanel->InitIcons(m_ArtefactsOnBelt);
		

	ROS()->force_mode	(IRender_ObjectSpecific::TRACE_ALL);

	m_pPhysics_support->in_NetSpawn	(e);
	character_physics_support()->movement()->ActivateBox	(0);
	if(E->m_holderID!=u16(-1))
	{ 
		character_physics_support()->movement()->DestroyCharacter();
	}
	if(m_bOutBorder)character_physics_support()->movement()->setOutBorder();
	r_torso_tgt_roll		= 0;

	r_model_yaw				= E->o_torso.yaw;
	r_torso.yaw				= E->o_torso.yaw;
	r_torso.pitch			= E->o_torso.pitch;
	r_torso.roll			= 0.0f;//E->o_Angle.z;

	unaffected_r_torso.yaw	= r_torso.yaw;
	unaffected_r_torso.pitch= r_torso.pitch;
	unaffected_r_torso.roll	= r_torso.roll;

	if( psActorFlags.test(AF_PSP) )
		cam_Set					(eacLookAt);
	else
		cam_Set					(eacFirstEye);

	cam_Active()->Set		(-E->o_torso.yaw,E->o_torso.pitch,0);//E->o_Angle.z);

	// *** movement state - respawn
	mstate_wishful			= 0;
	mstate_real				= 0;
	mstate_old				= 0;
	m_bJumpKeyPressed		= FALSE;

	NET_SavedAccel.set		(0,0,0);
	NET_WasInterpolating	= TRUE;

	setEnabled				(E->s_flags.is(M_SPAWN_OBJECT_LOCAL));

	shedule_register();

	if (!IsGameTypeSingle())
	{
		setEnabled(TRUE);
	}

	hit_slowmo				= 0.f;

	OnChangeVisual();
	//----------------------------------
	m_bAllowDeathRemove = false;

//	m_bHasUpdate = false;
	m_bInInterpolation = false;
	m_bInterpolate = false;

//	if (GameID() != GAME_SINGLE)
//	{
//		processing_activate();
//	}

#ifdef DEBUG
	LastPosS.clear();
	LastPosH.clear();
	LastPosL.clear();
#endif
//*
	
	SetDefaultVisualOutfit(cNameVisual());

	smart_cast<IKinematics*>(Visual())->CalculateBones();

	//--------------------------------------------------------------
	inventory().SetPrevActiveSlot(NO_ACTIVE_SLOT);


	//-------------------------------------
	m_States.clear();
	//-------------------------------------
	if (!g_Alive())
	{
		mstate_wishful	&=		~mcAnyMove;
		mstate_real		&=		~mcAnyMove;
		IKinematicsAnimated* K= smart_cast<IKinematicsAnimated*>(Visual());
		K->PlayCycle("death_init");

		
		//остановить звук т€желого дыхани€
		m_HeavyBreathSnd.stop();
	}
	
	typedef CClientSpawnManager::CALLBACK_TYPE	CALLBACK_TYPE;
	CALLBACK_TYPE	callback;
	callback.bind	(this,&CActor::on_requested_spawn);
	m_holder_id				= E->m_holderID;
	if (E->m_holderID != ALife::_OBJECT_ID(-1))
		if(!g_dedicated_server)
			Level().client_spawn_manager().add(E->m_holderID,ID(),callback);
	//F
	//-------------------------------------------------------------
	m_dwILastUpdateTime		= 0;

	if (IsGameTypeSingle()){
		Level().MapManager().AddMapLocation("actor_location",ID());
		Level().MapManager().AddMapLocation("actor_location_p",ID());

		m_game_task_manager	= xr_new<CGameTaskManager>();
		GameTaskManager().initialize(ID());

		m_statistic_manager = xr_new<CActorStatisticMgr>();
	}


	spatial.type |=STYPE_REACTTOSOUND;
	psHUD_Flags.set(HUD_WEAPON_RT,TRUE);

	return					TRUE;
}

void CActor::net_Destroy	()
{
	inherited::net_Destroy	();

	if (m_holder_id != ALife::_OBJECT_ID(-1))
		if(!g_dedicated_server)
			Level().client_spawn_manager().remove	(m_holder_id,ID());

	delete_data				(m_game_task_manager);
	delete_data				(m_statistic_manager);
	
	if(!g_dedicated_server)
		Level().MapManager		().RemoveMapLocationByObjectID(ID());

#pragma todo("Dima to MadMax : do not comment inventory owner net_Destroy!!!")
	CInventoryOwner::net_Destroy();
	cam_UnsetLadder();	
	character_physics_support()->movement()->DestroyCharacter();
	if(m_pPhysicsShell)			{
		m_pPhysicsShell->Deactivate();
		xr_delete<CPhysicsShell>(m_pPhysicsShell);
	};
	m_pPhysics_support->in_NetDestroy	();

	Device.remove_from_seq_parallel(fastdelegate::FastDelegate0<>(this, &CActor::PickupModeUpdateAll));

	xr_delete		(m_sndShockEffector);
	xr_delete		(pStatGraph);
	xr_delete		(m_pActorEffector);
	pCamBobbing		= NULL;
	
#ifdef DEBUG	
	LastPosS.clear();
	LastPosH.clear();
	LastPosL.clear();
#endif

	m_holder=NULL;
	m_holderID=u16(-1);
	
	m_ArtefactsOnBelt.clear();
	if (Level().CurrentViewEntity() == this)
		CurrentGameUI()->UIMainIngameWnd->get_hud_states()->UIArtefactPanel().InitIcons(m_ArtefactsOnBelt);

	SetDefaultVisualOutfit(NULL);
	

	if(g_actor == this) g_actor= NULL;

	Engine.Sheduler.Unregister	(this);
}

void CActor::net_Relcase	(CObject* O)
{
	
 	VERIFY(O);
	CGameObject* GO = smart_cast<CGameObject*>(O);
	if(GO&&m_pObjectWeLookingAt==GO){
		m_pObjectWeLookingAt=NULL;
	}
	CHolderCustom* HC=smart_cast<CHolderCustom*>(GO);
	if(HC&&HC==m_pVehicleWeLookingAt){
		m_pVehicleWeLookingAt=NULL;
	}
	if(HC&&HC==m_holder)
	{
		m_holder->detach_Actor();
		m_holder=NULL;
	}
	inherited::net_Relcase	(O);

	if (!g_dedicated_server)
		memory().remove_links(O);
	m_pPhysics_support->in_NetRelcase(O);
}

void	CActor::SetCallbacks()
{
	IKinematics* V		= smart_cast<IKinematics*>(Visual());
	VERIFY				(V);
	u16 spine0_bone		= V->LL_BoneID("bip01_spine");
	u16 spine1_bone		= V->LL_BoneID("bip01_spine1");
	u16 shoulder_bone	= V->LL_BoneID("bip01_spine2");
	u16 head_bone		= V->LL_BoneID("bip01_head");
	V->LL_GetBoneInstance(u16(spine0_bone)).set_callback	(bctCustom,Spin0Callback,this);
	V->LL_GetBoneInstance(u16(spine1_bone)).set_callback	(bctCustom,Spin1Callback,this);
	V->LL_GetBoneInstance(u16(shoulder_bone)).set_callback	(bctCustom,ShoulderCallback,this);
	V->LL_GetBoneInstance(u16(head_bone)).set_callback		(bctCustom,HeadCallback,this);
}
void	CActor::ResetCallbacks()
{
	IKinematics* V		= smart_cast<IKinematics*>(Visual());
	VERIFY				(V);
	u16 spine0_bone		= V->LL_BoneID("bip01_spine");
	u16 spine1_bone		= V->LL_BoneID("bip01_spine1");
	u16 shoulder_bone	= V->LL_BoneID("bip01_spine2");
	u16 head_bone		= V->LL_BoneID("bip01_head");
	V->LL_GetBoneInstance(u16(spine0_bone)).reset_callback	();
	V->LL_GetBoneInstance(u16(spine1_bone)).reset_callback	();
	V->LL_GetBoneInstance(u16(shoulder_bone)).reset_callback();
	V->LL_GetBoneInstance(u16(head_bone)).reset_callback	();
}

void	CActor::OnChangeVisual()
{
///	inherited::OnChangeVisual();
	{
		CPhysicsShell* tmp_shell=PPhysicsShell();
		PPhysicsShell()=NULL;
		inherited::OnChangeVisual();
		PPhysicsShell()=tmp_shell;
		tmp_shell=NULL;
	}

	IKinematicsAnimated* V	= smart_cast<IKinematicsAnimated*>(Visual());
	if (V){
		SetCallbacks		();
		m_anims->Create		(V);
		m_vehicle_anims->Create			(V);
		CDamageManager::reload(*cNameSect(),"damage",pSettings);
		//-------------------------------------------------------------------------------
		m_head				= smart_cast<IKinematics*>(Visual())->LL_BoneID("bip01_head");
		m_r_hand			= smart_cast<IKinematics*>(Visual())->LL_BoneID(pSettings->r_string(*cNameSect(),"weapon_bone0"));
		m_l_finger1			= smart_cast<IKinematics*>(Visual())->LL_BoneID(pSettings->r_string(*cNameSect(),"weapon_bone1"));
		m_r_finger2			= smart_cast<IKinematics*>(Visual())->LL_BoneID(pSettings->r_string(*cNameSect(),"weapon_bone2"));
		//-------------------------------------------------------------------------------
		m_neck				= smart_cast<IKinematics*>(Visual())->LL_BoneID("bip01_neck");
		m_l_clavicle		= smart_cast<IKinematics*>(Visual())->LL_BoneID("bip01_l_clavicle");
		m_r_clavicle		= smart_cast<IKinematics*>(Visual())->LL_BoneID("bip01_r_clavicle");
		m_spine2			= smart_cast<IKinematics*>(Visual())->LL_BoneID("bip01_spine2");
		m_spine1			= smart_cast<IKinematics*>(Visual())->LL_BoneID("bip01_spine1");
		m_spine				= smart_cast<IKinematics*>(Visual())->LL_BoneID("bip01_spine");
		//-------------------------------------------------------------------------------
		reattach_items();
		//-------------------------------------------------------------------------------
		m_pPhysics_support->in_ChangeVisual();
		//-------------------------------------------------------------------------------
		SetCallbacks		();
		//-------------------------------------------------------------------------------
		m_current_head.invalidate	();
		m_current_legs.invalidate	();
		m_current_torso.invalidate	();
		m_current_legs_blend		= NULL;
		m_current_torso_blend		= NULL;
		m_current_jump_blend		= NULL;
	}
};

void	CActor::ChangeVisual			( shared_str NewVisual )
{
	if (!NewVisual.size()) return;
	if (cNameVisual().size() )
	{
		if (cNameVisual() == NewVisual) return;
	}

	cNameVisual_set(NewVisual);

	g_SetAnimation			(mstate_real);
	Visual()->dcast_PKinematics()->CalculateBones_Invalidate();
	Visual()->dcast_PKinematics()->CalculateBones();
};

void ACTOR_DEFS::net_update::lerp(ACTOR_DEFS::net_update& A, ACTOR_DEFS::net_update& B, float f)
{
//	float invf		= 1.f-f;
//	// 
//	o_model			= angle_lerp	(A.o_model,B.o_model,		f);
//	o_torso.yaw		= angle_lerp	(A.o_torso.yaw,B.o_torso.yaw,f);
//	o_torso.pitch	= angle_lerp	(A.o_torso.pitch,B.o_torso.pitch,f);
//	o_torso.roll	= angle_lerp	(A.o_torso.roll,B.o_torso.roll,f);
//	p_pos.lerp		(A.p_pos,B.p_pos,f);
//	p_accel			= (f<0.5f)?A.p_accel:B.p_accel;
//	p_velocity.lerp	(A.p_velocity,B.p_velocity,f);
//	mstate			= (f<0.5f)?A.mstate:B.mstate;
//	weapon			= (f<0.5f)?A.weapon:B.weapon;
//	fHealth			= invf*A.fHealth+f*B.fHealth;
//	fArmor			= invf*A.fArmor+f*B.fArmor;
//	weapon			= (f<0.5f)?A.weapon:B.weapon;
}

void CActor::save(NET_Packet &output_packet)
{
	inherited::save(output_packet);
	CInventoryOwner::save(output_packet);
	output_packet.w_u8(u8(m_bOutBorder));
}

void CActor::load(IReader &input_packet)
{
	inherited::load(input_packet);
	CInventoryOwner::load(input_packet);
	m_bOutBorder=!!(input_packet.r_u8());
}

#ifdef DEBUG
void dbg_draw_piramid (Fvector pos, Fvector dir, float size, float xdir, u32 color)
{
	
	Fvector p0, p1, p2, p3, p4;
	p0.set(size, size, 0.0f);
	p1.set(-size, size, 0.0f);
	p2.set(-size, -size, 0.0f);
	p3.set(size, -size, 0.0f);
	p4.set(0, 0, size*4);
	
	bool Double = false;
	Fmatrix t; t.identity();
	if (_valid(dir) && dir.square_magnitude()>0.01f)
	{		
		t.k.normalize	(dir);
		Fvector::generate_orthonormal_basis(t.k, t.j, t.i);		
	}
	else
	{
		t.rotateY(xdir);		
		Double = true;
	}
	t.c.set(pos);

//	Level().debug_renderer().draw_line(t, p0, p1, color);
//	Level().debug_renderer().draw_line(t, p1, p2, color);
//	Level().debug_renderer().draw_line(t, p2, p3, color);
//	Level().debug_renderer().draw_line(t, p3, p0, color);

//	Level().debug_renderer().draw_line(t, p0, p4, color);
//	Level().debug_renderer().draw_line(t, p1, p4, color);
//	Level().debug_renderer().draw_line(t, p2, p4, color);
//	Level().debug_renderer().draw_line(t, p3, p4, color);
	
	if (!Double)
	{
		DRender->dbg_DrawTRI(t, p0, p1, p4, color);
		DRender->dbg_DrawTRI(t, p1, p2, p4, color);
		DRender->dbg_DrawTRI(t, p2, p3, p4, color);
		DRender->dbg_DrawTRI(t, p3, p0, p4, color);
	}
	else
	{
//		Fmatrix scale;
//		scale.scale(0.8f, 0.8f, 0.8f);
//		t.mulA_44(scale);
//		t.c.set(pos);

		Level().debug_renderer().draw_line(t, p0, p1, color);
		Level().debug_renderer().draw_line(t, p1, p2, color);
		Level().debug_renderer().draw_line(t, p2, p3, color);
		Level().debug_renderer().draw_line(t, p3, p0, color);

		Level().debug_renderer().draw_line(t, p0, p4, color);
		Level().debug_renderer().draw_line(t, p1, p4, color);
		Level().debug_renderer().draw_line(t, p2, p4, color);
		Level().debug_renderer().draw_line(t, p3, p4, color);
	};	
};

void	CActor::OnRender_Network()
{
	//RCache.OnFrameEnd();
	DRender->OnFrameEnd();

	//-----------------------------------------------------------------------------------------------------
	float size = 0.2f;
	
//	dbg_draw_piramid(Position(), m_PhysicMovementControl->GetVelocity(), size/2, -r_model_yaw, color_rgba(255, 255, 255, 255));
	//-----------------------------------------------------------------------------------------------------
	if (g_Alive())
	{
		if (dbg_net_Draw_Flags.test(dbg_draw_autopickupbox))
		{
			Fvector bc; bc.add(Position(), m_AutoPickUp_AABB_Offset);
			Fvector bd = m_AutoPickUp_AABB;

			Level().debug_renderer().draw_aabb			(bc, bd.x, bd.y, bd.z, color_rgba(0, 255, 0, 255));
		}
		
		IKinematics* V		= smart_cast<IKinematics*>(Visual());
		if (dbg_net_Draw_Flags.test(dbg_draw_actor_alive) && V)
		{
			if (this != Level().CurrentViewEntity() || cam_active != eacFirstEye)
			{
				/*
				u16 BoneCount = V->LL_BoneCount();
				for (u16 i=0; i<BoneCount; i++)
				{
					Fobb BoneOBB = V->LL_GetBox(i);
					Fmatrix BoneMatrix; BoneOBB.xform_get(BoneMatrix);
					Fmatrix BoneMatrixRes; BoneMatrixRes.mul(V->LL_GetTransform(i), BoneMatrix);
					BoneMatrix.mul(XFORM(), BoneMatrixRes);
					Level().debug_renderer().draw_obb(BoneMatrix, BoneOBB.m_halfsize, color_rgba(0, 255, 0, 255));
				};
				*/
				CCF_Skeleton* Skeleton = smart_cast<CCF_Skeleton*>(collidable.model);
				if (Skeleton){
					Skeleton->_dbg_refresh();

					const CCF_Skeleton::ElementVec& Elements = Skeleton->_GetElements();
					for (CCF_Skeleton::ElementVec::const_iterator I=Elements.begin(); I!=Elements.end(); I++){
						if (!I->valid())		continue;
						switch (I->type){
							case SBoneShape::stBox:{
								Fmatrix M;
								M.invert			(I->b_IM);
								Fvector h_size		= I->b_hsize;
								Level().debug_renderer().draw_obb	(M, h_size, color_rgba(0, 255, 0, 255));
							}break;
							case SBoneShape::stCylinder:{
								Fmatrix M;
								M.c.set				(I->c_cylinder.m_center);
								M.k.set				(I->c_cylinder.m_direction);
								Fvector				h_size;
								h_size.set			(I->c_cylinder.m_radius,I->c_cylinder.m_radius,I->c_cylinder.m_height*0.5f);
								Fvector::generate_orthonormal_basis(M.k,M.j,M.i);
								Level().debug_renderer().draw_obb	(M, h_size, color_rgba(0, 127, 255, 255));
							}break;
							case SBoneShape::stSphere:{
								Fmatrix				l_ball;
								l_ball.scale		(I->s_sphere.R, I->s_sphere.R, I->s_sphere.R);
								l_ball.translate_add(I->s_sphere.P);
								Level().debug_renderer().draw_ellipse(l_ball, color_rgba(0, 255, 0, 255));
							}break;
						};
					};					
				}
			};
		};

		if (!(dbg_net_Draw_Flags.is_any(dbg_draw_actor_dead)))
			return;
		
		dbg_draw_piramid(Position(), character_physics_support()->movement()->GetVelocity(), size, -r_model_yaw, color_rgba(128, 255, 128, 255));
		dbg_draw_piramid(IStart.Pos, IStart.Vel, size, -IStart.o_model, color_rgba(255, 0, 0, 255));
//		Fvector tmp, tmp1; tmp1.set(0, .1f, 0);
//		dbg_draw_piramid(tmp.add(IStartT.Pos, tmp1), IStartT.Vel, size, -IStartT.o_model, color_rgba(155, 0, 0, 155));
		dbg_draw_piramid(IRec.Pos, IRec.Vel, size, -IRec.o_model, color_rgba(0, 0, 255, 255));
//		dbg_draw_piramid(tmp.add(IRecT.Pos, tmp1), IRecT.Vel, size, -IRecT.o_model, color_rgba(0, 0, 155, 155));
		dbg_draw_piramid(IEnd.Pos, IEnd.Vel, size, -IEnd.o_model, color_rgba(0, 255, 0, 255));
//		dbg_draw_piramid(tmp.add(IEndT.Pos, tmp1), IEndT.Vel, size, -IEndT.o_model, color_rgba(0, 155, 0, 155));
		dbg_draw_piramid(NET_Last.p_pos, NET_Last.p_velocity, size*3/4, -NET_Last.o_model, color_rgba(255, 255, 255, 255));
		
		Fmatrix MS, MH, ML, *pM = NULL;
		ML.translate(0, 0.2f, 0);
		MS.translate(0, 0.2f, 0);
		MH.translate(0, 0.2f, 0);

		Fvector point0S, point1S, point0H, point1H, point0L, point1L, *ppoint0 = NULL, *ppoint1 = NULL;
		Fvector tS, tH;
		u32	cColor = 0, sColor = 0;
		VIS_POSITION*	pLastPos = NULL;

		switch (g_cl_InterpolationType)
		{
		case 0: ppoint0 = &point0L; ppoint1 = &point1L; cColor = color_rgba(0, 255, 0, 255); sColor = color_rgba(128, 255, 128, 255); pM = &ML; pLastPos = &LastPosL; break;
		case 1: ppoint0 = &point0S; ppoint1 = &point1S; cColor = color_rgba(0, 0, 255, 255); sColor = color_rgba(128, 128, 255, 255); pM = &MS; pLastPos = &LastPosS; break;
		case 2: ppoint0 = &point0H; ppoint1 = &point1H; cColor = color_rgba(255, 0, 0, 255); sColor = color_rgba(255, 128, 128, 255); pM = &MH; pLastPos = &LastPosH; break;
		}

		//drawing path trajectory
		float c = 0;
		for (int i=0; i<11; i++)
		{
			c = float(i) * 0.1f;
			for (u32 k=0; k<3; k++)
			{
				point1S[k] = c*(c*(c*SCoeff[k][0]+SCoeff[k][1])+SCoeff[k][2])+SCoeff[k][3];
				point1H[k] = c*(c*(c*HCoeff[k][0]+HCoeff[k][1])+HCoeff[k][2])+HCoeff[k][3];
				point1L[k] = IStart.Pos[k] + c*(IEnd.Pos[k]-IStart.Pos[k]);
			};
			if (i!=0)
			{
				Level().debug_renderer().draw_line(*pM, *ppoint0, *ppoint1, cColor);
			};
			point0S.set(point1S);
			point0H.set(point1H);
			point0L.set(point1L);
		};

		//drawing speed vectors
		for (int i=0; i<2; i++)
		{
			c = float(i);
			for (u32 k=0; k<3; k++)
			{
				point1S[k] = c*(c*(c*SCoeff[k][0]+SCoeff[k][1])+SCoeff[k][2])+SCoeff[k][3];
				point1H[k] = c*(c*(c*HCoeff[k][0]+HCoeff[k][1])+HCoeff[k][2])+HCoeff[k][3];

				tS[k] = (c*c*SCoeff[k][0]*3+c*SCoeff[k][1]*2+SCoeff[k][2])/3; // сокрость из формулы в 3 раза превышает скорость при расчете коэффициентов !!!!
				tH[k] = (c*c*HCoeff[k][0]*3+c*HCoeff[k][1]*2+HCoeff[k][2]); 
			};

			point0S.add(tS, point1S);
			point0H.add(tH, point1H);

			if (g_cl_InterpolationType > 0)
			{
				Level().debug_renderer().draw_line(*pM, *ppoint0, *ppoint1, sColor);
			}
		}

		//draw interpolation history curve
		if (!pLastPos->empty())
		{
			Fvector Pos1, Pos2;
			VIS_POSITION_it It = pLastPos->begin();
			Pos1 = *It;
			for (; It != pLastPos->end(); It++)
			{
				Pos2 = *It;

				Level().debug_renderer().draw_line	(*pM, Pos1, Pos2, cColor);
				Level().debug_renderer().draw_aabb	(Pos2, size/5, size/5, size/5, sColor);
				Pos1 = *It;
			};
		};

		Fvector PH, PS;
		PH.set(IPosH); PH.y += 1;
		PS.set(IPosS); PS.y += 1;
//		Level().debug_renderer().draw_aabb			(PS, size, size, size, color_rgba(128, 128, 255, 255));
//		Level().debug_renderer().draw_aabb			(PH, size, size, size, color_rgba(255, 128, 128, 255));
		/////////////////////////////////////////////////////////////////////////////////
	}
	else
	{
		if (!(dbg_net_Draw_Flags.is_any(dbg_draw_actor_dead))) return;

		IKinematics* V		= smart_cast<IKinematics*>(Visual());
		if (dbg_net_Draw_Flags.test(dbg_draw_actor_alive) && V)
		{
			u16 BoneCount = V->LL_BoneCount();
			for (u16 i=0; i<BoneCount; i++)
			{
				Fobb BoneOBB = V->LL_GetBox(i);
				Fmatrix BoneMatrix; BoneOBB.xform_get(BoneMatrix);
				Fmatrix BoneMatrixRes; BoneMatrixRes.mul(V->LL_GetTransform(i), BoneMatrix);
				BoneMatrix.mul(XFORM(), BoneMatrixRes);
				Level().debug_renderer().draw_obb(BoneMatrix, BoneOBB.m_halfsize, color_rgba(0, 255, 0, 255));
			};
		};

		if (!m_States.empty())
		{
			u32 NumBones = m_States.size();
			for (u32 i=0; i<NumBones; i++)
			{
				SPHNetState state = m_States[i];			

				Fvector half_dim;
				half_dim.x = 0.2f;
				half_dim.y = 0.1f;
				half_dim.z = 0.1f;

				u32 Color = color_rgba(255, 0, 0, 255);

				Fmatrix M;
				
				M = Fidentity;
				M.rotation(state.quaternion);
				M.translate_add(state.position);
				Level().debug_renderer().draw_obb				(M, half_dim, Color);

				if (!PHGetSyncItem(u16(i))) continue;
				PHGetSyncItem(u16(i))->get_State(state);

				Color = color_rgba(0, 255, 0, 255);
				M = Fidentity;
				M.rotation(state.quaternion);
				M.translate_add(state.position);
				Level().debug_renderer().draw_obb				(M, half_dim, Color);
			};
		}
		else
		{
			if (!g_Alive() && PHGetSyncItemsNumber() > 2)
			{
				u16 NumBones = PHGetSyncItemsNumber();
				for (u16 i=0; i<NumBones; i++)
				{
					SPHNetState state;// = m_States[i];
					PHGetSyncItem(i)->get_State(state);

					Fmatrix M;
					M = Fidentity;
					M.rotation(state.quaternion);
					M.translate_add(state.position);

					Fvector half_dim;
					half_dim.x = 0.2f;
					half_dim.y = 0.1f;
					half_dim.z = 0.1f;

					u32 Color = color_rgba(0, 255, 0, 255);
					Level().debug_renderer().draw_obb				(M, half_dim, Color);
				};
				//-----------------------------------------------------------------
				Fvector min,max;

				min.set(F_MAX,F_MAX,F_MAX);
				max.set(-F_MAX,-F_MAX,-F_MAX);
				/////////////////////////////////////
				for(u16 i=0;i<NumBones;i++)
				{
					SPHNetState state;
					PHGetSyncItem(i)->get_State(state);

					Fvector& p=state.position;
					UpdateLimits (p, min, max);

					Fvector px =state.linear_vel;
					px.div(10.0f);
					px.add(state.position);
					UpdateLimits (px, min, max);
				};

				NET_Packet PX;
				for(u16 i=0;i<NumBones;i++)
				{
					SPHNetState state;
					PHGetSyncItem(i)->get_State(state);

					PX.wPos = 0;
					w_vec_q8(PX,state.position,min,max);
					w_qt_q8(PX,state.quaternion);
//					w_vec_q8(PX,state.linear_vel,min,max);

					PX.r_pos = 0;
					r_vec_q8(PX,state.position,min,max);
					r_qt_q8(PX,state.quaternion);
//					r_vec_q8(PX,state.linear_vel,min,max);
					//===============================================
					Fmatrix M;
					M = Fidentity;
					M.rotation(state.quaternion);
					M.translate_add(state.position);

					Fvector half_dim;
					half_dim.x = 0.2f;
					half_dim.y = 0.1f;
					half_dim.z = 0.1f;

					u32 Color = color_rgba(255, 0, 0, 255);
					Level().debug_renderer().draw_obb				(M, half_dim, Color);
				};	
				Fvector LC, LS;
				LC.add(min, max); LC.div(2.0f);
				LS.sub(max, min); LS.div(2.0f);

				Level().debug_renderer().draw_aabb			(LC, LS.x, LS.y, LS.z, color_rgba(255, 128, 128, 255));
				//-----------------------------------------------------------------
			};
		}
	}
};

#endif

BOOL CActor::net_SaveRelevant()
{
	return TRUE;
}

bool				CActor::InventoryAllowSprint			()
{
	PIItem pActiveItem = inventory().ActiveItem();
	if (pActiveItem && !pActiveItem->IsSprintAllowed())
	{
		return false;
	};
	PIItem pOutfitItem = inventory().ItemFromSlot(OUTFIT_SLOT);
	if (pOutfitItem && !pOutfitItem->IsSprintAllowed())
	{
		return false;
	}
	return true;
}

void			CActor::On_B_NotCurrentEntity		()
{
	inventory().Items_SetCurrentEntityHud(false);
};