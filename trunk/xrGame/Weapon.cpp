// Weapon.cpp: implementation of the CWeapon class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Weapon.h"
#include "ParticlesObject.h"
#include "HUDManager.h"
#include "WeaponHUD.h"
#include "entity_alive.h"

#include "inventory.h"
#include "xrserver_objects_alife_items.h"

#include "actor.h"
#include "level.h"

#include "xr_level_controller.h"
#include "game_cl_base.h"
#include "..\include\xrRender\Kinematics.h"
#include "ai_object_location.h"
#include "CameraEffector.h"
#include "clsid_game.h"
#include "gamepersistent.h"
#include "object_broker.h"
#include "../xr_3da/igame_persistent.h"
#include "EffectorFall.h"
#include "ui/UIStatix.h"

#define WEAPON_REMOVE_TIME		60000
#define ROTATION_TIME			0.25f

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWeapon::CWeapon(LPCSTR name)
{
	SetState				(eHidden);
	SetNextState			(eHidden);
	m_sub_state				= eSubstateReloadBegin;
	m_bTriStateReload		= false;
	m_idle_state = eIdle;
	SetDefaults				();

	m_Offset.identity		();
	m_StrapOffset.identity	();

	iAmmoCurrent			= -1;
	m_dwAmmoCurrentCalcFrame= 0;

	iAmmoElapsed			= -1;
	iMagazineSize			= -1;
	m_ammoType				= 0;
	m_ammoName				= nullptr;

	eHandDependence			= hdNone;

	m_fZoomFactor			= g_fov;
	m_fZoomRotationFactor	= 0.f;


	m_pAmmo					= nullptr;


	m_pFlameParticles2		= nullptr;
	m_sFlameParticles2		= nullptr;


	m_fCurrentCartirdgeDisp = 1.f;

	m_strap_bone0		    = nullptr;
	m_strap_bone1			= nullptr;
	m_StrapOffset.identity	();
	m_strapped_mode			= false;
	m_can_be_strapped		= false;
	m_ef_main_weapon_type	= static_cast<u32>(-1);
	m_ef_weapon_type		= static_cast<u32>(-1);
	m_UIScope				= nullptr;
	m_set_next_ammoType_on_reload = static_cast<u32>(-1);
}

CWeapon::~CWeapon		()
{
	xr_delete	(m_UIScope);
}

void CWeapon::Hit(SHit* pHDS)
{
	inherited::Hit(pHDS);
}



void CWeapon::UpdateXForm	()
{
	if (Device.dwFrame!=dwXF_Frame)
	{
		dwXF_Frame = Device.dwFrame;

		if (!H_Parent())	
			return;

		// Get access to entity and its visual
		auto*	E		= smart_cast<CEntityAlive*>(H_Parent());
		
		if(!E) 
		{
			if (!IsGameTypeSingle())
				UpdatePosition(H_Parent()->XFORM());
			return;
		}

		const auto* parent = smart_cast<const CInventoryOwner*>(E);
		if (parent && parent->use_simplified_visual())
			return;

		if (parent->attached(this))
			return;

		R_ASSERT		(E);
		auto*	V		= smart_cast<IKinematics*>	(E->Visual());
		VERIFY			(V);

		// Get matrices
		int				boneL,boneR,boneR2;
		E->g_WeaponBones(boneL,boneR,boneR2);
		if ((HandDependence() == hd1Hand) || (GetState() == eReload) || (!E->g_Alive()))
			boneL = boneR2;
		V->CalculateBones	();
		Fmatrix& mL			= V->LL_GetTransform(u16(boneL));
		Fmatrix& mR			= V->LL_GetTransform(u16(boneR));
		// Calculate
		Fmatrix				mRes;
		Fvector				R,D,N;
		D.sub				(mL.c,mR.c);	

		if(fis_zero(D.magnitude()))
		{
			mRes.set(E->XFORM());
			mRes.c.set(mR.c);
		}
		else
		{		
			D.normalize();
			R.crossproduct	(mR.j,D);

			N.crossproduct	(D,R);			
			N.normalize();

			mRes.set		(R,N,D,mR.c);
			mRes.mulA_43	(E->XFORM());
		}

		UpdatePosition	(mRes);
	}
}

void CWeapon::UpdateFireDependencies_internal()
{
	if (Device.dwFrame!=dwFP_Frame) 
	{
		dwFP_Frame			= Device.dwFrame;

		UpdateXForm			();

		if (GetHUDmode() && H_Parent())
		{
			// 1st person view - skeletoned size()
			auto* V			= smart_cast<IKinematics*>(m_pHUD->Visual());
			VERIFY					(V);
			V->CalculateBones		();

			// fire point&direction
			Fmatrix& fire_mat		= V->LL_GetTransform(u16(m_pHUD->FireBone()));
			Fmatrix& parent			= m_pHUD->Transform	();

			const Fvector& fp		= m_pHUD->FirePoint();
			const Fvector& fp2		= m_pHUD->FirePoint2();
			const Fvector& sp		= m_pHUD->ShellPoint();

			fire_mat.transform_tiny	(m_firedeps.vLastFP,fp);
			parent.transform_tiny	(m_firedeps.vLastFP);
			fire_mat.transform_tiny	(m_firedeps.vLastFP2,fp2);
			parent.transform_tiny	(m_firedeps.vLastFP2);
		
			fire_mat.transform_tiny	(m_firedeps.vLastSP,sp);
			parent.transform_tiny	(m_firedeps.vLastSP);

			m_firedeps.vLastFD.set	(0.f,0.f,1.f);
			parent.transform_dir	(m_firedeps.vLastFD);

			m_firedeps.m_FireParticlesXForm.identity();
			m_firedeps.m_FireParticlesXForm.k.set(m_firedeps.vLastFD);
			Fvector::generate_orthonormal_basis_normalized(m_firedeps.m_FireParticlesXForm.k,
									m_firedeps.m_FireParticlesXForm.j, m_firedeps.m_FireParticlesXForm.i);
		} else {
			// 3rd person or no parent
			Fmatrix& parent			= XFORM();
			Fvector& fp				= vLoadedFirePoint;
			Fvector& fp2			= vLoadedFirePoint2;
			Fvector& sp				= vLoadedShellPoint;

			parent.transform_tiny	(m_firedeps.vLastFP,fp);
			parent.transform_tiny	(m_firedeps.vLastFP2,fp2);
			parent.transform_tiny	(m_firedeps.vLastSP,sp);
			
			m_firedeps.vLastFD.set	(0.f,0.f,1.f);
			parent.transform_dir	(m_firedeps.vLastFD);

			m_firedeps.m_FireParticlesXForm.set(parent);
		}
	}
}

void CWeapon::ForceUpdateFireParticles()
{
	if (!GetHUDmode())
	{//update particlesXFORM real bullet direction

		if (!H_Parent())		
			return;

		if(auto* io = smart_cast<CInventoryOwner*>(H_Parent()); !io->inventory().ActiveItem())
		{
				Log("current_state", GetState() );
				Log("next_state", GetNextState());
				Log("state_time", m_dwStateTime);
				Log("item_sect", cNameSect().c_str());
				Log("H_Parent", H_Parent()->cNameSect().c_str());
		}

		Fvector					p, d; 
		smart_cast<CEntity*>(H_Parent())->g_fireParams	(this, p,d);

		Fmatrix						_pxf;
		_pxf.k						= d;
		_pxf.i.crossproduct			(Fvector().set(0.0f,1.0f,0.0f),	_pxf.k);
		_pxf.j.crossproduct			(_pxf.k,		_pxf.i);
		_pxf.c						= XFORM().c;
		
		m_firedeps.m_FireParticlesXForm.set	(_pxf);

	}

}

void CWeapon::Load		(LPCSTR section)
{
	inherited::Load					(section);
	CShootingObject::Load			(section);

	
	if(pSettings->line_exist(section, "flame_particles_2"))
		m_sFlameParticles2 = pSettings->r_string(section, "flame_particles_2");

#ifdef DEBUG
	{
		Fvector				pos,ypr;
		pos					= pSettings->r_fvector3		(section,"position");
		ypr					= pSettings->r_fvector3		(section,"orientation");
		ypr.mul				(PI/180.f);

		m_Offset.setHPB			(ypr.x,ypr.y,ypr.z);
		m_Offset.translate_over	(pos);
	}

	m_StrapOffset			= m_Offset;
	if (pSettings->line_exist(section,"strap_position") && pSettings->line_exist(section,"strap_orientation")) {
		Fvector				pos,ypr;
		pos					= pSettings->r_fvector3		(section,"strap_position");
		ypr					= pSettings->r_fvector3		(section,"strap_orientation");
		ypr.mul				(PI/180.f);

		m_StrapOffset.setHPB			(ypr.x,ypr.y,ypr.z);
		m_StrapOffset.translate_over	(pos);
	}
#endif

	// load ammo classes
	m_ammoTypes.clear	(); 
	LPCSTR				S = pSettings->r_string(section,"ammo_class");
	if (S && S[0]) 
	{
		int				count		= _GetItemCount	(S);
		for (int it=0; it<count; ++it)
		{
			string128		_ammoItem;
			_GetItem				(S,it,_ammoItem);
			m_ammoTypes.push_back	(_ammoItem);
		}
		m_ammoName = pSettings->r_string(*m_ammoTypes[0],"inv_name_short");
	}
	else
		m_ammoName = nullptr;

	iAmmoElapsed		= pSettings->r_s32		(section,"ammo_elapsed"		);
	iMagazineSize		= pSettings->r_s32		(section,"ammo_mag_size"	);
	
	////////////////////////////////////////////////////
	// ��������� ��������

	//������������� ������ �� ����� ������
	camMaxAngle			= pSettings->r_float		(section,"cam_max_angle"	); 
	camMaxAngle			= deg2rad					(camMaxAngle);
	camRelaxSpeed		= pSettings->r_float		(section,"cam_relax_speed"	); 
	camRelaxSpeed		= deg2rad					(camRelaxSpeed);
	if (pSettings->line_exist(section, "cam_relax_speed_ai"))
	{
		camRelaxSpeed_AI		= pSettings->r_float		(section,"cam_relax_speed_ai"	); 
		camRelaxSpeed_AI		= deg2rad					(camRelaxSpeed_AI);
	}
	else
	{
		camRelaxSpeed_AI	= camRelaxSpeed;
	}
	
//	camDispersion		= pSettings->r_float		(section,"cam_dispersion"	); 
//	camDispersion		= deg2rad					(camDispersion);

	camMaxAngleHorz		= pSettings->r_float		(section,"cam_max_angle_horz"	); 
	camMaxAngleHorz		= deg2rad					(camMaxAngleHorz);
	camStepAngleHorz	= pSettings->r_float		(section,"cam_step_angle_horz"	); 
	camStepAngleHorz	= deg2rad					(camStepAngleHorz);	
	camDispertionFrac			= READ_IF_EXISTS(pSettings, r_float, section, "cam_dispertion_frac",	0.7f);
	//  [8/2/2005]
	//m_fParentDispersionModifier = READ_IF_EXISTS(pSettings, r_float, section, "parent_dispersion_modifier",1.0f);
	m_fPDM_disp_base			= READ_IF_EXISTS(pSettings, r_float, section, "PDM_disp_base",	1.0f);
	m_fPDM_disp_vel_factor		= READ_IF_EXISTS(pSettings, r_float, section, "PDM_disp_vel_factor",	1.0f);
	m_fPDM_disp_accel_factor	= READ_IF_EXISTS(pSettings, r_float, section, "PDM_disp_accel_factor",	1.0f);
	m_fPDM_disp_crouch			= READ_IF_EXISTS(pSettings, r_float, section, "PDM_disp_crouch",	1.0f);
	m_fPDM_disp_crouch_no_acc	= READ_IF_EXISTS(pSettings, r_float, section, "PDM_disp_crouch_no_acc",	1.0f);
	//  [8/2/2005]

	fireDispersionConditionFactor = pSettings->r_float(section,"fire_dispersion_condition_factor"); 
	misfireProbability			  = pSettings->r_float(section,"misfire_probability"); 
	misfireConditionK			  = READ_IF_EXISTS(pSettings, r_float, section, "misfire_condition_k",	1.0f);
	conditionDecreasePerShot	  = pSettings->r_float(section,"condition_shot_dec"); 
		
	vLoadedFirePoint	= pSettings->r_fvector3		(section,"fire_point"		);
	
	if(pSettings->line_exist(section,"fire_point2")) 
		vLoadedFirePoint2= pSettings->r_fvector3	(section,"fire_point2");
	else 
		vLoadedFirePoint2= vLoadedFirePoint;

	// hands
	eHandDependence		= EHandDependence(pSettings->r_s32(section,"hand_dependence"));
	m_bIsSingleHanded	= true;
	if (pSettings->line_exist(section, "single_handed"))
		m_bIsSingleHanded	= !!pSettings->r_bool(section, "single_handed");
	// 
	m_fMinRadius		= pSettings->r_float		(section,"min_radius");
	m_fMaxRadius		= pSettings->r_float		(section,"max_radius");


	// ���������� � ��������� ��������� � �� ������������ � ���������
	m_eScopeStatus			 = static_cast<ALife::EWeaponAddonStatus>(pSettings->r_s32(section,"scope_status"));
	m_eSilencerStatus		 = static_cast<ALife::EWeaponAddonStatus>(pSettings->r_s32(section,"silencer_status"));
	m_eGrenadeLauncherStatus = static_cast<ALife::EWeaponAddonStatus>(pSettings->r_s32(section,"grenade_launcher_status"));

	m_bZoomEnabled = !!pSettings->r_bool(section,"zoom_enabled");
	m_fZoomRotateTime = ROTATION_TIME;
	if(m_bZoomEnabled && m_pHUD) LoadZoomOffset(*hud_sect, "");

	if(m_eScopeStatus == ALife::eAddonAttachable)
	{
		m_sScopeName = pSettings->r_string(section,"scope_name");
		m_iScopeX = pSettings->r_s32(section,"scope_x");
		m_iScopeY = pSettings->r_s32(section,"scope_y");
	}

    
	if(m_eSilencerStatus == ALife::eAddonAttachable)
	{
		m_sSilencerName = pSettings->r_string(section,"silencer_name");
		m_iSilencerX = pSettings->r_s32(section,"silencer_x");
		m_iSilencerY = pSettings->r_s32(section,"silencer_y");
	}

    
	if(m_eGrenadeLauncherStatus == ALife::eAddonAttachable)
	{
		m_sGrenadeLauncherName = pSettings->r_string(section,"grenade_launcher_name");
		m_iGrenadeLauncherX = pSettings->r_s32(section,"grenade_launcher_x");
		m_iGrenadeLauncherY = pSettings->r_s32(section,"grenade_launcher_y");
	}

	InitAddons();

	//////////////////////////////////////
	//����� �������� ������ � ������
	if(pSettings->line_exist(section,"weapon_remove_time"))
		m_dwWeaponRemoveTime = pSettings->r_u32(section,"weapon_remove_time");
	else
		m_dwWeaponRemoveTime = WEAPON_REMOVE_TIME;
	//////////////////////////////////////
	if(pSettings->line_exist(section,"auto_spawn_ammo"))
		m_bAutoSpawnAmmo = pSettings->r_bool(section,"auto_spawn_ammo");
	else
		m_bAutoSpawnAmmo = TRUE;
	//////////////////////////////////////


	m_bHideCrosshairInZoom = true;
	if(pSettings->line_exist(hud_sect, "zoom_hide_crosshair"))
		m_bHideCrosshairInZoom = !!pSettings->r_bool(hud_sect, "zoom_hide_crosshair");	

	Fvector			def_dof;
	def_dof.set(-1, -1, -1);
	m_zoom_params.m_ZoomDof		= READ_IF_EXISTS(pSettings, r_fvector3, section, "zoom_dof", Fvector().set(-1,-1,-1));
	m_zoom_params.m_bZoomDofEnabled	= !def_dof.similar(m_zoom_params.m_ZoomDof);

	m_zoom_params.m_ReloadDof	= READ_IF_EXISTS(pSettings, r_fvector4, section, "reload_dof", Fvector4().set(-1,-1,-1,-1));

	m_zoom_params.m_ReloadEmptyDof = READ_IF_EXISTS(pSettings, r_fvector4, section, "reload_empty_dof", Fvector4().set(-1, -1, -1, -1));

	m_bHasTracers = READ_IF_EXISTS(pSettings, r_bool, section, "tracers", true);
	m_u8TracerColorID = READ_IF_EXISTS(pSettings, r_u8, section, "tracers_color_ID", static_cast<u8>(-1));

	string256						temp;
	for (int i=egdNovice; i<egdCount; ++i) {
		strconcat					(sizeof(temp),temp,"hit_probability_",get_token_name(difficulty_type_token,i));
		m_hit_probability[i]		= READ_IF_EXISTS(pSettings,r_float,section,temp,1.f);
	}
}

void CWeapon::LoadFireParams		(LPCSTR section, LPCSTR prefix)
{
	camDispersion		= pSettings->r_float		(section,"cam_dispersion"	); 
	camDispersion		= deg2rad					(camDispersion);

	if (pSettings->line_exist(section,"cam_dispersion_inc"))
	{
		camDispersionInc		= pSettings->r_float		(section,"cam_dispersion_inc"	); 
		camDispersionInc		= deg2rad					(camDispersionInc);
	}
	else
		camDispersionInc = 0;

	CShootingObject::LoadFireParams(section, prefix);
};

void CWeapon::LoadZoomOffset (LPCSTR section, LPCSTR prefix)
{
	string256 full_name;
	m_pHUD->SetZoomOffset(pSettings->r_fvector3	(hud_sect, strconcat(sizeof(full_name),full_name, prefix, "zoom_offset")));
	m_pHUD->SetZoomRotateX(pSettings->r_float	(hud_sect, strconcat(sizeof(full_name),full_name, prefix, "zoom_rotate_x")));
	m_pHUD->SetZoomRotateY(pSettings->r_float	(hud_sect, strconcat(sizeof(full_name),full_name, prefix, "zoom_rotate_y")));

	if(pSettings->line_exist(hud_sect, "zoom_rotate_time"))
		m_fZoomRotateTime = pSettings->r_float(hud_sect,"zoom_rotate_time");
}

BOOL CWeapon::net_Spawn		(CSE_Abstract* DC)
{
	BOOL bResult					= inherited::net_Spawn(DC);
	auto* e	= (CSE_Abstract*)(DC);
	auto* E	= smart_cast<CSE_ALifeItemWeapon*>(e);

	//iAmmoCurrent					= E->a_current;
	iAmmoElapsed					= E->a_elapsed;
	m_flagsAddOnState				= E->m_addon_flags.get();
	m_ammoType						= E->ammo_type;
	SetState						(E->wpn_state);
	SetNextState					(E->wpn_state);
	
	m_DefaultCartridge.Load(*m_ammoTypes[m_ammoType], static_cast<u8>(m_ammoType));	
	if(iAmmoElapsed) 
	{
		m_fCurrentCartirdgeDisp = m_DefaultCartridge.m_kDisp;
		for(int i = 0; i < iAmmoElapsed; ++i) 
			m_magazine.push_back(m_DefaultCartridge);
	}


	UpdateAddonsVisibility();
	InitAddons();


	m_dwWeaponIndependencyTime = 0;

	VERIFY((u32)iAmmoElapsed == m_magazine.size());
	m_bAmmoWasSpawned		= false;

	return bResult;
}

void CWeapon::net_Destroy	()
{
	inherited::net_Destroy	();

	//������� ������� ���������
	StopFlameParticles	();
	StopFlameParticles2	();
	StopLight			();
	Light_Destroy		();

	m_magazine.clear();
}

BOOL CWeapon::IsUpdating()
{	
	bool bIsActiveItem = m_pCurrentInventory && m_pCurrentInventory->ActiveItem()==this;
	return bIsActiveItem || bWorking || m_bPending || getVisible();
}

void CWeapon::net_Export(NET_Packet& P)
{
	inherited::net_Export	(P);

	P.w_float_q8			(m_fCondition,0.0f,1.0f);

	u8 need_upd				= IsUpdating() ? 1 : 0;
	P.w_u8					(need_upd);
	P.w_u16					(static_cast<u16>(iAmmoElapsed));
	P.w_u8					(m_flagsAddOnState);
	P.w_u8					(static_cast<u8>(m_ammoType));
	P.w_u8					(static_cast<u8>(GetState()));
	P.w_u8					(m_bZoomMode);
}

void CWeapon::save(NET_Packet &output_packet)
{
	inherited::save	(output_packet);
	save_data		(iAmmoElapsed,		output_packet);
	save_data		(m_flagsAddOnState, output_packet);
	save_data		(m_ammoType,		output_packet);
	save_data		(m_bZoomMode,		output_packet);
}

void CWeapon::load(IReader &input_packet)
{
	inherited::load	(input_packet);
	load_data		(iAmmoElapsed,		input_packet);
	load_data		(m_flagsAddOnState, input_packet);
	UpdateAddonsVisibility	();
	load_data		(m_ammoType,		input_packet);
	load_data		(m_bZoomMode,		input_packet);

	if (m_bZoomMode)	OnZoomIn();
		else			OnZoomOut();
}


void CWeapon::OnEvent(NET_Packet& P, u16 type) 
{
	switch (type)
	{
	case GE_ADDON_CHANGE:
		{
			P.r_u8					(m_flagsAddOnState);
			InitAddons();
			UpdateAddonsVisibility();
		}break;

	case GE_WPN_STATE_CHANGE:
		{
			u8				state;
			P.r_u8			(state);
			P.r_u8			(m_sub_state);		
//			u8 NewAmmoType = 
				P.r_u8();
			u8 AmmoElapsed = P.r_u8();
			u8 NextAmmo = P.r_u8();
			if (NextAmmo == static_cast<u8>(-1))
				m_set_next_ammoType_on_reload = static_cast<u32>(-1);
			else
				m_set_next_ammoType_on_reload = NextAmmo;

			if (OnClient()) 
				SetAmmoElapsed(static_cast<int>(AmmoElapsed));			
			OnStateSwitch	(static_cast<u32>(state));
		}
		break;
	default:
		{
			inherited::OnEvent(P,type);
		}break;
	}
};

void CWeapon::shedule_Update	(u32 dT)
{;	
	// Inherited
	inherited::shedule_Update	(dT);
}

void CWeapon::OnH_B_Independent	(bool just_before_destroy)
{
	RemoveShotEffector			();

	inherited::OnH_B_Independent(just_before_destroy);

	if (m_pHUD)
		m_pHUD->Hide			();

	//��������� ������������� ��� �������� ��� ���
	FireEnd();
	m_bPending = false;
	SwitchState(eIdle);

	m_strapped_mode				= false;
	SetHUDmode					(FALSE);
	m_bZoomMode					= false;
	UpdateXForm					();

}

void CWeapon::OnH_A_Independent	()
{
	m_dwWeaponIndependencyTime = Level().timeServer();
	inherited::OnH_A_Independent();
	Light_Destroy				();
};

void CWeapon::OnH_A_Chield		()
{
	inherited::OnH_A_Chield		();

	UpdateAddonsVisibility		();
};

void CWeapon::OnActiveItem ()
{
	inherited::OnActiveItem		();
	//���� �� ����������� � ������ ���� � �����
	SetState					(eIdle);
	SetNextState				(eIdle);
	if (m_pHUD) m_pHUD->Show	();
}

void CWeapon::OnHiddenItem ()
{
	inherited::OnHiddenItem();
	if (m_pHUD)	m_pHUD->Hide ();
	SetState					(eHidden);
	SetNextState				(eHidden);
	m_set_next_ammoType_on_reload	= static_cast<u32>(-1);
}


void CWeapon::OnH_B_Chield		()
{
	m_dwWeaponIndependencyTime = 0;
	inherited::OnH_B_Chield		();

	OnZoomOut					();
	m_set_next_ammoType_on_reload	= static_cast<u32>(-1);
}

u8 CWeapon::idle_state()
{
	auto* actor = smart_cast<CActor*>(H_Parent());

	if (actor)
	{
		u32 st = actor->get_state();
		if (st & mcSprint)
			return eSubstateIdleSprint;
		else if (st & mcAnyAction && !(st & mcJump) && !(st & mcFall))
			return eSubstateIdleMoving;
	}

	return eIdle;
}

void CWeapon::UpdateCL		()
{
	inherited::UpdateCL		();
	UpdateHUDAddonsVisibility();
	//��������� �� ��������
	UpdateLight				();

	//���������� ��������
	UpdateFlameParticles	();
	UpdateFlameParticles2	();

	if(!IsGameTypeSingle())
		make_Interpolation		();
	
	VERIFY(smart_cast<IKinematics*>(Visual()));


	if (GetState() == eIdle) {
		auto state = idle_state();
		if (m_idle_state != state) {
			m_idle_state = state;
			if (GetNextState() != eMagEmpty && GetNextState() != eReload)
			{
				SwitchState(eIdle);
			}
		}
	}
	else
		m_idle_state = eIdle;
}


void CWeapon::renderable_Render		()
{
	UpdateXForm				();

	//���������� ���������

	RenderLight				();	

	//���� �� � ������ ���������, �� ��� HUD �������� �� ����
	if(IsZoomed() && !IsRotatingToZoom() && ZoomTexture())
		m_bRenderHud = false;
	else
		m_bRenderHud = true;

	inherited::renderable_Render		();
}

void CWeapon::signal_HideComplete()
{
	if(H_Parent()) 
		setVisible(FALSE);
	m_bPending = false;
	if(m_pHUD) m_pHUD->Hide();
}

void CWeapon::SetDefaults()
{
	bWorking2			= false;
	m_bPending			= false;

	m_flags.set			(FUsingCondition, TRUE);
	bMisfire			= false;
	m_flagsAddOnState	= 0;
	m_bZoomMode			= false;
}

void CWeapon::UpdatePosition(const Fmatrix& trans)
{
	Position().set		(trans.c);
	XFORM().mul			(trans,m_strapped_mode ? m_StrapOffset : m_Offset);
	VERIFY				(!fis_zero(DET(renderable.xform)));
}


bool CWeapon::Action(s32 cmd, u32 flags) 
{
	if(inherited::Action(cmd, flags)) return true;

	
	switch(cmd) 
	{
		case kWPN_FIRE: 
			{
				//���� ������ ���-�� ������, �� ������ �� ������
				{				
					if(flags&CMD_START) 
					{
						if(IsPending())		return false;
						FireStart			();
					}
					else 
						FireEnd();
				}

			} 
			return true;
		case kWPN_NEXT: 
			{
				if(IsPending() || OnClient()) 
				{
					return false;
				}

				if (Core.Features.test(xrCore::Feature::lock_reload_in_sprint) && ParentIsActor() && g_actor->get_state() & mcSprint)
					return true;

				if(flags & CMD_START) 
				{
					u32 l_newType = m_ammoType;
					bool b1,b2;
					do 
					{
						l_newType = (l_newType+1)%m_ammoTypes.size();
						b1 = l_newType != m_ammoType;
						b2 = unlimited_ammo() ? false : (!m_pCurrentInventory->GetAny(*m_ammoTypes[l_newType]));						
					} while( b1 && b2);

					if(l_newType != m_ammoType) 
					{
						m_set_next_ammoType_on_reload = l_newType;

						if(OnServer()) Reload();
					}
				}
			} 
            return true;

		case kWPN_ZOOM:
			if(IsZoomEnabled())
			{
                if(flags&CMD_START && !IsPending())
					OnZoomIn();
                else if(IsZoomed())
					OnZoomOut();
				return true;
			}
		    else 
				return false;

		case kWPN_ZOOM_INC:
		case kWPN_ZOOM_DEC:
			if(IsZoomEnabled() && IsZoomed())
			{
				if(cmd==kWPN_ZOOM_INC)  ZoomInc();
				else					ZoomDec();
				return true;
			}else
				return false;
	}
	return false;
}

void CWeapon::SpawnAmmo(u32 boxCurr, LPCSTR ammoSect, u32 ParentID) 
{
	if(m_ammoTypes.empty())			
		return;
	if (OnClient())					
		return;
	m_bAmmoWasSpawned				= true;
	
	int l_type						= 0;
	l_type							%= m_ammoTypes.size();

	if(!ammoSect) ammoSect			= *m_ammoTypes[l_type]; 
	
	++l_type; 
	l_type							%= m_ammoTypes.size();

	CSE_Abstract *D					= F_entity_Create(ammoSect);

	if (D->m_tClassID==CLSID_OBJECT_AMMO	||
		D->m_tClassID==CLSID_OBJECT_A_M209	||
		D->m_tClassID==CLSID_OBJECT_A_VOG25	||
		D->m_tClassID==CLSID_OBJECT_A_OG7B)
	{	
		auto *l_pA		= smart_cast<CSE_ALifeItemAmmo*>(D);
		R_ASSERT					(l_pA);
		l_pA->m_boxSize				= static_cast<u16>(pSettings->r_s32(ammoSect, "box_size"));
		D->s_name					= ammoSect;
		D->set_name_replace			("");
		D->s_gameid					= static_cast<u8>(GameID());
		D->s_RP						= 0xff;
		D->ID						= 0xffff;
		if (ParentID == 0xffffffff)	
			D->ID_Parent			= H_Parent()->ID();
		else
			D->ID_Parent			= static_cast<u16>(ParentID);

		D->ID_Phantom				= 0xffff;
		D->s_flags.assign			(M_SPAWN_OBJECT_LOCAL);
		D->RespawnTime				= 0;
		l_pA->m_tNodeID				= g_dedicated_server ? static_cast<u32>(-1) : ai_location().level_vertex_id();

		if(boxCurr == 0xffffffff) 	
			boxCurr					= l_pA->m_boxSize;

		while(boxCurr) 
		{
			l_pA->a_elapsed			= static_cast<u16>(boxCurr > l_pA->m_boxSize ? l_pA->m_boxSize : boxCurr);
			NET_Packet				P;
			D->Spawn_Write			(P, TRUE);
			Level().Send			(P,net_flags(TRUE));

			if(boxCurr > l_pA->m_boxSize) 
				boxCurr				-= l_pA->m_boxSize;
			else 
				boxCurr				= 0;
		}
	};
	F_entity_Destroy				(D);
}

int CWeapon::GetAmmoCurrent(bool use_item_to_spawn) const
{
	int l_count = iAmmoElapsed;
	if(!m_pCurrentInventory) return l_count;

	//���� �� ������ ������ ����������
	if(m_pCurrentInventory->ModifyFrame()<=m_dwAmmoCurrentCalcFrame)
		return l_count + iAmmoCurrent;

 	m_dwAmmoCurrentCalcFrame = Device.dwFrame;
	iAmmoCurrent = 0;

	for(int i = 0; i < static_cast<int>(m_ammoTypes.size()); ++i) 
	{
		LPCSTR l_ammoType = *m_ammoTypes[i];

		for(auto l_it = m_pCurrentInventory->m_belt.begin(); m_pCurrentInventory->m_belt.end() != l_it; ++l_it) 
		{
			auto *l_pAmmo = smart_cast<CWeaponAmmo*>(*l_it);

			if(l_pAmmo && !xr_strcmp(l_pAmmo->cNameSect(), l_ammoType)) 
			{
				iAmmoCurrent = iAmmoCurrent + l_pAmmo->m_boxCurr;
			}
		}

		for(auto l_it = m_pCurrentInventory->m_ruck.begin(); m_pCurrentInventory->m_ruck.end() != l_it; ++l_it) 
		{
			auto *l_pAmmo = smart_cast<CWeaponAmmo*>(*l_it);
			if(l_pAmmo && !xr_strcmp(l_pAmmo->cNameSect(), l_ammoType)) 
			{
				iAmmoCurrent = iAmmoCurrent + l_pAmmo->m_boxCurr;
			}
		}

		if (!use_item_to_spawn)
			continue;

		if (!inventory_owner().item_to_spawn())
			continue;

		iAmmoCurrent += inventory_owner().ammo_in_box_to_spawn();
	}
	return l_count + iAmmoCurrent;
}

float CWeapon::GetConditionMisfireProbability() const
{
	if( GetCondition()>0.95f ) return 0.0f;

	float mis = misfireProbability+powf(1.f-GetCondition(), 3.f)*misfireConditionK;
	clamp(mis,0.0f,0.99f);
	return mis;
}

BOOL CWeapon::CheckForMisfire()
{
	if (OnClient()) 
		return FALSE;

	float rnd = ::Random.randF(0.f,1.f);
	float mp = GetConditionMisfireProbability();
	if(rnd < mp)
	{
		if (iAmmoElapsed == 0)
			return false;

		FireEnd();

		bMisfire = true;
		SwitchState(eMisfire);		
		
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CWeapon::IsMisfire() const
{	
	return bMisfire;
}
void CWeapon::Reload()
{
	OnZoomOut();
}


bool CWeapon::IsGrenadeLauncherAttached() const
{
	return (ALife::eAddonAttachable == m_eGrenadeLauncherStatus &&
			0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonGrenadeLauncher)) || 
			ALife::eAddonPermanent == m_eGrenadeLauncherStatus;
}

bool CWeapon::IsScopeAttached() const
{
	return (ALife::eAddonAttachable == m_eScopeStatus &&
			0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonScope)) || 
			ALife::eAddonPermanent == m_eScopeStatus;

}

bool CWeapon::IsSilencerAttached() const
{
	return (ALife::eAddonAttachable == m_eSilencerStatus &&
			0 != (m_flagsAddOnState&CSE_ALifeItemWeapon::eWeaponAddonSilencer)) || 
			ALife::eAddonPermanent == m_eSilencerStatus;
}

bool CWeapon::GrenadeLauncherAttachable()
{
	return (ALife::eAddonAttachable == m_eGrenadeLauncherStatus);
}
bool CWeapon::ScopeAttachable()
{
	return (ALife::eAddonAttachable == m_eScopeStatus);
}
bool CWeapon::SilencerAttachable()
{
	return (ALife::eAddonAttachable == m_eSilencerStatus);
}

LPCSTR wpn_scope				= "wpn_scope";
LPCSTR wpn_silencer				= "wpn_silencer";
LPCSTR wpn_grenade_launcher		= "wpn_grenade_launcher";
LPCSTR wpn_launcher				= "wpn_launcher";

void CWeapon::UpdateHUDAddonsVisibility()
{//actor only
	if(H_Parent() != Level().CurrentEntity())				
		return;
	if(m_pHUD->IsHidden())									
		return;

	auto* pHudVisual = smart_cast<IKinematics*>(m_pHUD->Visual());
	VERIFY(pHudVisual);
	if (H_Parent() != Level().CurrentEntity()) 
		pHudVisual	= nullptr;

	if (!pHudVisual)return;
	u16  bone_id;

	bone_id = pHudVisual->LL_BoneID(wpn_scope);
	if(ScopeAttachable())
	{
		VERIFY2(bone_id!=BI_NONE,"there are no scope bone.");
		if(IsScopeAttached())
		{
			if(FALSE==pHudVisual->LL_GetBoneVisible		(bone_id))
				pHudVisual->LL_SetBoneVisible			(bone_id,TRUE,TRUE);
		}else{
			if(pHudVisual->LL_GetBoneVisible			(bone_id))
				pHudVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);
		}
	}
	if(m_eScopeStatus == ALife::eAddonDisabled && bone_id!=BI_NONE && 
		pHudVisual->LL_GetBoneVisible(bone_id) )
		pHudVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);
	else
	if(m_eScopeStatus == ALife::eAddonPermanent && bone_id!=BI_NONE && 
		!pHudVisual->LL_GetBoneVisible(bone_id) )
		pHudVisual->LL_SetBoneVisible			(bone_id,TRUE,TRUE);


	bone_id = pHudVisual->LL_BoneID(wpn_silencer);
	if(SilencerAttachable())
	{
		VERIFY2(bone_id!=BI_NONE,"there are no silencer bone.");
		if(IsSilencerAttached())
		{
			if(FALSE==pHudVisual->LL_GetBoneVisible		(bone_id))
				pHudVisual->LL_SetBoneVisible			(bone_id,TRUE,TRUE);
		}else{
			if(pHudVisual->LL_GetBoneVisible			(bone_id))
				pHudVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);
		}
	}
	if(m_eSilencerStatus == ALife::eAddonDisabled && bone_id!=BI_NONE &&
		pHudVisual->LL_GetBoneVisible(bone_id) )
		pHudVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);
	else
	if(m_eSilencerStatus == ALife::eAddonPermanent && bone_id != BI_NONE &&
		!pHudVisual->LL_GetBoneVisible(bone_id))
		pHudVisual->LL_SetBoneVisible			(bone_id,TRUE,TRUE);


	bone_id = pHudVisual->LL_BoneID(wpn_grenade_launcher);
	if(GrenadeLauncherAttachable())
	{
		if(bone_id==BI_NONE)
			bone_id = pHudVisual->LL_BoneID(wpn_launcher);

		VERIFY2(bone_id!=BI_NONE,"there are no grenade launcher bone.");
		if(IsGrenadeLauncherAttached())
		{
			if(FALSE==pHudVisual->LL_GetBoneVisible		(bone_id))
				pHudVisual->LL_SetBoneVisible			(bone_id,TRUE,TRUE);
		}
		else
		{
			if(pHudVisual->LL_GetBoneVisible			(bone_id))
				pHudVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);
		}
	}
	if(m_eGrenadeLauncherStatus == ALife::eAddonDisabled && bone_id!=BI_NONE &&
		pHudVisual->LL_GetBoneVisible(bone_id) )
		pHudVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);
	else
	if(m_eGrenadeLauncherStatus == ALife::eAddonPermanent && bone_id!=BI_NONE &&
		!pHudVisual->LL_GetBoneVisible(bone_id) )
		pHudVisual->LL_SetBoneVisible			(bone_id,TRUE,TRUE);


}

void CWeapon::UpdateAddonsVisibility()
{
	auto* pWeaponVisual = smart_cast<IKinematics*>(Visual());
	R_ASSERT(pWeaponVisual);

	u16  bone_id;
	UpdateHUDAddonsVisibility								();	

	bone_id = pWeaponVisual->LL_BoneID					(wpn_scope);
	if(ScopeAttachable())
	{
		if(IsScopeAttached())
		{
			if(FALSE==pWeaponVisual->LL_GetBoneVisible		(bone_id))
			pWeaponVisual->LL_SetBoneVisible				(bone_id,TRUE,TRUE);
		}else{
			if(pWeaponVisual->LL_GetBoneVisible				(bone_id))
				pWeaponVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);
		}
	}
	if(m_eScopeStatus == ALife::eAddonDisabled && bone_id!=BI_NONE &&
		pWeaponVisual->LL_GetBoneVisible(bone_id) )

		pWeaponVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);

	bone_id = pWeaponVisual->LL_BoneID					(wpn_silencer);
	if(SilencerAttachable())
	{
		if(IsSilencerAttached()){
			if(FALSE==pWeaponVisual->LL_GetBoneVisible		(bone_id))
				pWeaponVisual->LL_SetBoneVisible			(bone_id,TRUE,TRUE);
		}else{
			if( pWeaponVisual->LL_GetBoneVisible				(bone_id))
				pWeaponVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);
		}
	}
	if(m_eSilencerStatus == ALife::eAddonDisabled && bone_id!=BI_NONE &&
		pWeaponVisual->LL_GetBoneVisible(bone_id) )

		pWeaponVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);

	bone_id = pWeaponVisual->LL_BoneID					(wpn_launcher);
	if(GrenadeLauncherAttachable())
	{
		if(IsGrenadeLauncherAttached())
		{
			if(FALSE==pWeaponVisual->LL_GetBoneVisible		(bone_id))
				pWeaponVisual->LL_SetBoneVisible			(bone_id,TRUE,TRUE);
		}else{
			if(pWeaponVisual->LL_GetBoneVisible				(bone_id))
				pWeaponVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);
		}
	}
	if(m_eGrenadeLauncherStatus == ALife::eAddonDisabled && bone_id!=BI_NONE &&
		pWeaponVisual->LL_GetBoneVisible(bone_id) )

		pWeaponVisual->LL_SetBoneVisible			(bone_id,FALSE,TRUE);
	

	pWeaponVisual->CalculateBones_Invalidate				();
	pWeaponVisual->CalculateBones							();
}

bool CWeapon::Activate() 
{
	UpdateAddonsVisibility();
	return inherited::Activate();
}

void CWeapon::InitAddons()
{
}

float CWeapon::CurrentZoomFactor	()
{
	return IsScopeAttached() ? m_fScopeZoomFactor : m_fIronSightZoomFactor;
};

void CWeapon::OnZoomIn()
{
	m_bZoomMode = true;
	m_fZoomFactor = CurrentZoomFactor();
	StopHudInertion();

	if (m_zoom_params.m_bZoomDofEnabled && !IsScopeAttached())
		GamePersistent().SetEffectorDOF(m_zoom_params.m_ZoomDof);

	if (GetHUDmode())
		GamePersistent().SetPickableEffectorDOF(true);
}

void CWeapon::OnZoomOut()
{
	m_bZoomMode = false;
	m_fZoomFactor = g_fov;

	StartHudInertion();

    GamePersistent().RestoreEffectorDOF();

	if (GetHUDmode())
		GamePersistent().SetPickableEffectorDOF(false);
}

CUIStaticItem* CWeapon::ZoomTexture()
{
	if (UseScopeTexture())
		return m_UIScope;
	else
		return nullptr;
}

void CWeapon::SwitchState(u32 S)
{
	if (OnClient()) return;

	SetNextState		( S );	// Very-very important line of code!!! :)
	if (CHudItem::object().Local() && !CHudItem::object().getDestroy()/* && (S!=NEXT_STATE)*/ 
		&& m_pCurrentInventory && OnServer())	
	{
		// !!! Just single entry for given state !!!
		NET_Packet		P;
		CHudItem::object().u_EventGen		(P,GE_WPN_STATE_CHANGE,CHudItem::object().ID());
		P.w_u8			(u8(S));
		P.w_u8			(u8(m_sub_state));
		P.w_u8			(u8(m_ammoType& 0xff));
		P.w_u8			(u8(iAmmoElapsed & 0xff));
		P.w_u8			(u8(m_set_next_ammoType_on_reload & 0xff));
		CHudItem::object().u_EventSend		(P, net_flags(TRUE, TRUE, FALSE, TRUE));
	}
}

void CWeapon::OnMagazineEmpty	()
{
	VERIFY((u32)iAmmoElapsed == m_magazine.size());
}


void CWeapon::reinit			()
{
	CShootingObject::reinit		();
	CHudItemObject::reinit			();
}

void CWeapon::reload			(LPCSTR section)
{
	CShootingObject::reload		(section);
	CHudItemObject::reload			(section);
	
	m_can_be_strapped			= true;
	m_strapped_mode				= false;
	
	if (pSettings->line_exist(section,"strap_bone0"))
		m_strap_bone0			= pSettings->r_string(section,"strap_bone0");
	else
		m_can_be_strapped		= false;
	
	if (pSettings->line_exist(section,"strap_bone1"))
		m_strap_bone1			= pSettings->r_string(section,"strap_bone1");
	else
		m_can_be_strapped		= false;

	if (m_eScopeStatus == ALife::eAddonAttachable) {
		m_addon_holder_range_modifier	= READ_IF_EXISTS(pSettings,r_float,m_sScopeName,"holder_range_modifier",m_holder_range_modifier);
		m_addon_holder_fov_modifier		= READ_IF_EXISTS(pSettings,r_float,m_sScopeName,"holder_fov_modifier",m_holder_fov_modifier);
	}
	else {
		m_addon_holder_range_modifier	= m_holder_range_modifier;
		m_addon_holder_fov_modifier		= m_holder_fov_modifier;
	}


	{
		Fvector				pos,ypr;
		pos					= pSettings->r_fvector3		(section,"position");
		ypr					= pSettings->r_fvector3		(section,"orientation");
		ypr.mul				(PI/180.f);

		m_Offset.setHPB			(ypr.x,ypr.y,ypr.z);
		m_Offset.translate_over	(pos);
	}

	m_StrapOffset			= m_Offset;
	if (pSettings->line_exist(section,"strap_position") && pSettings->line_exist(section,"strap_orientation")) {
		Fvector				pos,ypr;
		pos					= pSettings->r_fvector3		(section,"strap_position");
		ypr					= pSettings->r_fvector3		(section,"strap_orientation");
		ypr.mul				(PI/180.f);

		m_StrapOffset.setHPB			(ypr.x,ypr.y,ypr.z);
		m_StrapOffset.translate_over	(pos);
	}
	else
		m_can_be_strapped	= false;

	m_ef_main_weapon_type	= READ_IF_EXISTS(pSettings,r_u32,section,"ef_main_weapon_type",u32(-1));
	m_ef_weapon_type		= READ_IF_EXISTS(pSettings,r_u32,section,"ef_weapon_type",u32(-1));
}

void CWeapon::create_physic_shell()
{
	CPhysicsShellHolder::create_physic_shell();
}

void CWeapon::activate_physic_shell()
{
	CPhysicsShellHolder::activate_physic_shell();
}

void CWeapon::setup_physic_shell()
{
	CPhysicsShellHolder::setup_physic_shell();
}

int		g_iWeaponRemove = 1;

bool CWeapon::NeedToDestroyObject()	const
{
	if (H_Parent()) return false;
	if (g_iWeaponRemove == -1) return false;
	if (g_iWeaponRemove == 0) return true;
	return (TimePassedAfterIndependant() > m_dwWeaponRemoveTime);
}

ALife::_TIME_ID	 CWeapon::TimePassedAfterIndependant()	const
{
	if(!H_Parent() && m_dwWeaponIndependencyTime != 0)
		return Level().timeServer() - m_dwWeaponIndependencyTime;
	else
		return 0;
}

bool CWeapon::can_kill	() const
{
	if (GetAmmoCurrent(true) || m_ammoTypes.empty())
		return				(true);

	return					(false);
}

CInventoryItem *CWeapon::can_kill	(CInventory *inventory) const
{
	if (GetAmmoElapsed() || m_ammoTypes.empty())
		return				(const_cast<CWeapon*>(this));

	TIItemContainer::iterator I = inventory->m_all.begin();
	TIItemContainer::iterator E = inventory->m_all.end();
	for ( ; I != E; ++I) {
		CInventoryItem	*inventory_item = smart_cast<CInventoryItem*>(*I);
		if (!inventory_item)
			continue;
		
		xr_vector<shared_str>::const_iterator	i = std::find(m_ammoTypes.begin(),m_ammoTypes.end(),inventory_item->object().cNameSect());
		if (i != m_ammoTypes.end())
			return			(inventory_item);
	}

	return					(0);
}

const CInventoryItem *CWeapon::can_kill	(const xr_vector<const CGameObject*> &items) const
{
	if (m_ammoTypes.empty())
		return				(this);

	xr_vector<const CGameObject*>::const_iterator I = items.begin();
	xr_vector<const CGameObject*>::const_iterator E = items.end();
	for ( ; I != E; ++I) {
		const CInventoryItem	*inventory_item = smart_cast<const CInventoryItem*>(*I);
		if (!inventory_item)
			continue;

		xr_vector<shared_str>::const_iterator	i = std::find(m_ammoTypes.begin(),m_ammoTypes.end(),inventory_item->object().cNameSect());
		if (i != m_ammoTypes.end())
			return			(inventory_item);
	}

	return					(0);
}

bool CWeapon::ready_to_kill	() const
{
	return					(
		!IsMisfire() && 
		((GetState() == eIdle) || (GetState() == eFire) || (GetState() == eFire2)) && 
		GetAmmoElapsed()
	);
}


void CWeapon::UpdateHudAdditonal		(Fmatrix& trans)
{
	CActor* pActor = smart_cast<CActor*>(H_Parent());
	if(!pActor) return;

	if(		(pActor->IsZoomAimingMode() && m_fZoomRotationFactor<=1.f) ||
			(!pActor->IsZoomAimingMode() && m_fZoomRotationFactor>0.f))
	{
		Fmatrix hud_rotation;
		hud_rotation.identity();
		hud_rotation.rotateX(m_pHUD->ZoomRotateX()*m_fZoomRotationFactor);

		Fmatrix hud_rotation_y;
		hud_rotation_y.identity();
		hud_rotation_y.rotateY(m_pHUD->ZoomRotateY()*m_fZoomRotationFactor);
		hud_rotation.mulA_43(hud_rotation_y);

		Fvector offset = m_pHUD->ZoomOffset();
		offset.mul					(m_fZoomRotationFactor);
		hud_rotation.translate_over	(offset);
		trans.mulB_43				(hud_rotation);

		if(pActor->IsZoomAimingMode())
			m_fZoomRotationFactor += Device.fTimeDelta/m_fZoomRotateTime;
		else
			m_fZoomRotationFactor -= Device.fTimeDelta/m_fZoomRotateTime;
		clamp(m_fZoomRotationFactor, 0.f, 1.f);
	}
}

void	CWeapon::SetAmmoElapsed	(int ammo_count)
{
	iAmmoElapsed				= ammo_count;

	
	if (const u32 uAmmo	= static_cast<u32>(iAmmoElapsed); uAmmo != m_magazine.size())
	{
		if (uAmmo > m_magazine.size())
		{
			CCartridge			l_cartridge; 
			l_cartridge.Load	(*m_ammoTypes[m_ammoType], u8(m_ammoType));
			while (uAmmo > m_magazine.size())
				m_magazine.push_back(l_cartridge);
		}
		else
		{
			while (uAmmo < m_magazine.size())
				m_magazine.pop_back();
		}
	}
}

u32	CWeapon::ef_main_weapon_type	() const
{
	VERIFY	(m_ef_main_weapon_type != u32(-1));
	return	(m_ef_main_weapon_type);
}

u32	CWeapon::ef_weapon_type	() const
{
	VERIFY	(m_ef_weapon_type != u32(-1));
	return	(m_ef_weapon_type);
}

bool CWeapon::IsNecessaryItem	    (const shared_str& item_sect)
{
	return (std::find(m_ammoTypes.begin(), m_ammoTypes.end(), item_sect) != m_ammoTypes.end() );
}

void CWeapon::modify_holder_params		(float &range, float &fov) const
{
	if (!IsScopeAttached()) {
		inherited::modify_holder_params	(range,fov);
		return;
	}
	range	*= m_addon_holder_range_modifier;
	fov		*= m_addon_holder_fov_modifier;
}

void CWeapon::OnDrawUI()
{
	if(IsZoomed() && ZoomHideCrosshair()){
		if(ZoomTexture() && !IsRotatingToZoom()){
			ZoomTexture()->SetPos	(0,0);
			ZoomTexture()->SetRect	(0,0,UI_BASE_WIDTH, UI_BASE_HEIGHT);
			ZoomTexture()->Render	();
		}
	}
}

bool CWeapon::unlimited_ammo() const
{ 
	if (GameID() == GAME_SINGLE	)
		return psActorFlags.test(AF_UNLIMITEDAMMO) && 
				m_DefaultCartridge.m_flags.test(CCartridge::cfCanBeUnlimited); 

	return (GameID()!=GAME_ARTEFACTHUNT) && 
		m_DefaultCartridge.m_flags.test(CCartridge::cfCanBeUnlimited); 
			
};

LPCSTR	CWeapon::GetCurrentAmmo_ShortName() const
{
	if (m_magazine.empty()) 
		return ("");
	const CCartridge &l_cartridge = m_magazine.back();
	return *(l_cartridge.m_InvShortName);
}

float CWeapon::Weight()
{
	float res = CInventoryItemObject::Weight();
	if(IsGrenadeLauncherAttached()&&GetGrenadeLauncherName().size()){
		res += pSettings->r_float(GetGrenadeLauncherName(),"inv_weight");
	}
	if(IsScopeAttached()&&GetScopeName().size()){
		res += pSettings->r_float(GetScopeName(),"inv_weight");
	}
	if(IsSilencerAttached()&&GetSilencerName().size()){
		res += pSettings->r_float(GetSilencerName(),"inv_weight");
	}
	
	if(iAmmoElapsed)
	{
		const float w		= pSettings->r_float(*m_ammoTypes[m_ammoType],"inv_weight");
		const float bs	= pSettings->r_float(*m_ammoTypes[m_ammoType],"box_size");

		res			+= w*(static_cast<float>(iAmmoElapsed)/bs);
	}
	return res;
}
void CWeapon::Hide		()
{
	if(IsGameTypeSingle())
		SwitchState(eHiding);
	else
		SwitchState(eHidden);

	OnZoomOut();
}

void CWeapon::Show		()
{
	SwitchState(eShowing);
}

bool CWeapon::show_crosshair()
{
	return !IsZoomed();
}

bool CWeapon::show_indicators()
{
	return ! ( IsZoomed() && ZoomTexture() );
}

float CWeapon::GetConditionToShow	() const
{
	return	(GetCondition());
}

BOOL CWeapon::ParentMayHaveAimBullet	()
{
	CObject* O=H_Parent();
	auto* EA=smart_cast<CEntityAlive*>(O);
	return EA->cast_actor() != nullptr;
}

BOOL CWeapon::ParentIsActor	()
{
	CObject* O = H_Parent();
	auto* EA=smart_cast<CEntityAlive*>(O);
	return EA->cast_actor() != nullptr;
}

const float &CWeapon::hit_probability() const
{
	VERIFY					((g_SingleGameDifficulty >= egdNovice) && (g_SingleGameDifficulty <= egdMaster)); 
	return					(m_hit_probability[egdNovice]);
}

void CWeapon::OnStateSwitch(u32 S)
{
	inherited::OnStateSwitch(S);
	m_dwAmmoCurrentCalcFrame = 0;

	if (GetState() == eReload)
	{
		if (iAmmoElapsed == 0)
		{
			if (H_Parent() == Level().CurrentEntity() && !fsimilar(m_zoom_params.m_ReloadEmptyDof.w, -1.0f))
			{
				CActor* current_actor = smart_cast<CActor*>(H_Parent());
				if (current_actor)
					current_actor->Cameras().AddCamEffector(xr_new<CEffectorDOF>(m_zoom_params.m_ReloadEmptyDof));
			}
		}
		else
		{
			if (H_Parent() == Level().CurrentEntity() && !fsimilar(m_zoom_params.m_ReloadDof.w, -1.0f))
			{
				CActor* current_actor = smart_cast<CActor*>(H_Parent());
				if (current_actor)
					current_actor->Cameras().AddCamEffector(xr_new<CEffectorDOF>(m_zoom_params.m_ReloadDof));
			}
		}
	}
}