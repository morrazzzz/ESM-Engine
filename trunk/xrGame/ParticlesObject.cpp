//----------------------------------------------------
// file: PSObject.cpp
//----------------------------------------------------
#include "stdafx.h"
#pragma hdrstop

#include "ParticlesObject.h"
#include "../xr_3da/defines.h"
#include "..\Include\xrRender\RenderVisual.h"
#include "../Include/xrRender/ParticleCustom.h"
#include "../xr_3da/render.h"
#include "../xr_3da/IGame_Persistent.h"

constexpr Fvector zero_vel = { 0.f,0.f,0.f };

CParticlesObject::CParticlesObject(std::string_view p_name, bool bAutoRemove, bool destroy_on_game_load) :
	inherited(destroy_on_game_load)
{
	Init(p_name, nullptr, bAutoRemove);
}

void CParticlesObject::Init(std::string_view p_name, IRender_Sector* S, bool bAutoRemove)
{
	m_bLooped = false;
	m_bStopping = false;
	m_bAutoRemove = bAutoRemove;
	float time_limit = 0.0f;

	// create visual
	renderable.visual = Render->model_CreateParticles(p_name.data());
	VERIFY(renderable.visual);
	IParticleCustom* V = smart_cast<IParticleCustom*>(renderable.visual);  
	VERIFY(V);
	time_limit = V->GetTimeLimit();

	if (time_limit > 0.f)
	{
		m_iLifeTime = iFloor(time_limit * 1000.f);
	}
	else
	{
		if (bAutoRemove)
			R_ASSERT3(!m_bAutoRemove, "Can't set auto-remove flag for looped particle system.", p_name.data());
		else
		{
			m_iLifeTime = 0;
			m_bLooped = true;
		}
	}


	// spatial
	spatial.type = 0;
	spatial.sector = S;

	// sheduled
	shedule.t_min = 20;
	shedule.t_max = 50;
	shedule_register();

	dwLastTime = Device.dwTimeGlobal;
}

//----------------------------------------------------
CParticlesObject::~CParticlesObject()
{
}

void CParticlesObject::UpdateSpatial()
{
	// spatial	(+ workaround occasional bug inside particle-system)
	vis_data& vis = renderable.visual->getVisData();

	if (_valid(vis.sphere))
	{
		Fvector	P;	float	R;
		renderable.xform.transform_tiny(P, vis.sphere.P);
		R = vis.sphere.R;

		if (!spatial.type)	
		{
			// First 'valid' update - register
			spatial.type			= STYPE_RENDERABLE;
			spatial.sphere.set		(P,R);
			spatial_register		();
		}
		else 
		{
			BOOL	bMove			= FALSE;
			if		(!P.similar(spatial.sphere.P,EPS_L*10.f))		bMove	= TRUE;
			if		(!fsimilar(R,spatial.sphere.R,0.15f))			bMove	= TRUE;
			if		(bMove)			{
				spatial.sphere.set	(P, R);
				spatial_move		();
			}
		}
	}
}

const shared_str CParticlesObject::Name()
{
	IParticleCustom* V	= smart_cast<IParticleCustom*>(renderable.visual); 
	R_ASSERT(V);
	return V->Name();
}

//----------------------------------------------------
void CParticlesObject::Play		(bool hudMode)
{
	IParticleCustom* V			= smart_cast<IParticleCustom*>(renderable.visual); 
	R_ASSERT(V);
	V->SetHudMode				(hudMode);
	V->Play						();
	dwLastTime					= Device.dwTimeGlobal-33ul;
	PerformAllTheWork			();
	m_bStopping					= false;
}

void CParticlesObject::play_at_pos(const Fvector& pos, BOOL xform)
{
	IParticleCustom* V			= smart_cast<IParticleCustom*>(renderable.visual); 
	R_ASSERT(V);
	Fmatrix m; m.translate		(pos); 
	V->UpdateParent				(m,zero_vel,xform);
	V->Play						();
	dwLastTime					= Device.dwTimeGlobal-33ul;
	PerformAllTheWork();
	m_bStopping = false;
}

void CParticlesObject::Stop		(BOOL bDefferedStop)
{
	IParticleCustom* V			= smart_cast<IParticleCustom*>(renderable.visual); 
	R_ASSERT(V);
	V->Stop						(bDefferedStop);
	m_bStopping					= true;
}

void CParticlesObject::UpdateParticles()
{
	if (m_bDead) 
		return;

	u32 dt = Device.dwTimeGlobal - dwLastTime;
	if (dt) {
		IParticleCustom* V = smart_cast<IParticleCustom*>(renderable.visual); 
		R_ASSERT(V);
		V->OnFrame(dt);
		dwLastTime = Device.dwTimeGlobal;
	}
}

void CParticlesObject::PerformAllTheWork()
{
	// Update
	u32 dt = Device.dwTimeGlobal - dwLastTime;
	if (dt) {
		IParticleCustom* V = smart_cast<IParticleCustom*>(renderable.visual); 
		R_ASSERT(V);
		V->OnFrame(dt);
		dwLastTime = Device.dwTimeGlobal;
	}
	UpdateSpatial();
}

void CParticlesObject::SetXFORM(const Fmatrix& m)
{
	IParticleCustom* V = smart_cast<IParticleCustom*>(renderable.visual); 
	R_ASSERT(V);
	V->UpdateParent(m, zero_vel, TRUE);
	renderable.xform.set(m);
	UpdateSpatial();
}

void CParticlesObject::UpdateParent		(const Fmatrix& m, const Fvector& vel)
{
	IParticleCustom* V	= smart_cast<IParticleCustom*>(renderable.visual); 
	R_ASSERT(V);
	V->UpdateParent		(m,vel,FALSE);
	UpdateSpatial		();
}

void CParticlesObject::renderable_Render	()
{
	R_ASSERT(renderable.visual);
	::Render->set_Transform	(&renderable.xform);
	::Render->add_Visual	(renderable.visual);
}
bool CParticlesObject::IsAutoRemove			()
{
	if(m_bAutoRemove) return true;
	else return false;
}
void CParticlesObject::SetAutoRemove		(bool auto_remove)
{
	VERIFY(m_bStopping || !IsLooped());
	m_bAutoRemove = auto_remove;
}

//играются ли партиклы, отличается от PSI_Alive, тем что после
//остановки Stop партиклы могут еще доигрывать анимацию IsPlaying = true
bool CParticlesObject::IsPlaying()
{
	IParticleCustom* V = smart_cast<IParticleCustom*>(renderable.visual);
	R_ASSERT(V);
	return V->IsPlaying();
}
