//----------------------------------------------------
// file: TempObject.cpp
//----------------------------------------------------
#include "stdafx.h"
#pragma hdrstop

#include "ps_instance.h"
#include "IGame_Persistent.h"
#include "../Include/xrRender/RenderVisual.h"

CPS_Instance::CPS_Instance			(bool destroy_on_game_load)	:
	ISpatial				(g_SpatialSpace),
	m_destroy_on_game_load	(destroy_on_game_load)
{
	g_pGamePersistent->ps_active.insert		(this);
	g_pGamePersistent->ps_needtoupdate.emplace_back(this);

	renderable.pROS_Allowed					= FALSE;

	m_iLifeTime								= int_max;
	m_bAutoRemove							= TRUE;
	m_bDead									= FALSE;
}
extern ENGINE_API BOOL						g_bRendering; 

//----------------------------------------------------
CPS_Instance::~CPS_Instance					()
{
	VERIFY									(!g_bRendering);
	xr_set<CPS_Instance*>::iterator it		= g_pGamePersistent->ps_active.find(this);
	VERIFY									(it!=g_pGamePersistent->ps_active.end());
	g_pGamePersistent->ps_active.erase		(it);

	std::erase_if(g_pGamePersistent->ps_needtoupdate, [this](CPS_Instance* particle) { return particle == this; });

	xr_vector<CPS_Instance*>::iterator it2	= std::find( g_pGamePersistent->ps_destroy.begin(),
													g_pGamePersistent->ps_destroy.end(), this);

	VERIFY									(it2==g_pGamePersistent->ps_destroy.end());

	spatial_unregister						();
	shedule_unregister						();
}
//----------------------------------------------------
const Fvector& CPS_Instance::PositionParticle()
{
	return renderable.visual->getVisData().sphere.P;
}

void CPS_Instance::shedule_Update	(u32 dt)
{
	if (renderable.pROS)			::Render->ros_destroy	(renderable.pROS);	//. particles doesn't need ROS

	ISheduled::shedule_Update		(dt);
	m_iLifeTime						-= dt;

	// remove???
	if (m_bDead)					return;
	if (m_bAutoRemove && m_iLifeTime<=0)
		PSI_destroy					();
}

float CPS_Instance::shedule_Scale()
{
	return Device.vCameraPosition.distance_to(PositionParticle()) / 200.f;
}
//----------------------------------------------------
void CPS_Instance::PSI_destroy		()
{
	m_bDead								= TRUE;
	m_iLifeTime							= 0;
	g_pGamePersistent->ps_destroy.push_back	(this);
}
//----------------------------------------------------
void CPS_Instance::PSI_internal_delete		()
{
	delete this;
}
