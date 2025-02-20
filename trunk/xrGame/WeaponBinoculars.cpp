#include "stdafx.h"
#include "WeaponBinoculars.h"

#include "xr_level_controller.h"

#include "level.h"
#include "WeaponBinocularsVision.h"
#include "object_broker.h"
#include "inventory.h"

void CWeaponBinoculars::Load	(LPCSTR section)
{
	inherited::Load(section);

	// Sounds
	m_sounds.LoadSound(section, "snd_zoomin",  "sndZoomIn",		false, SOUND_TYPE_ITEM_USING);
	m_sounds.LoadSound(section, "snd_zoomout", "sndZoomOut",	false, SOUND_TYPE_ITEM_USING);
	if (pSettings->r_bool(section, "vision_present"))
		m_zoom_params.m_pVision = new CBinocularsVision(cNameSect().c_str());
}


bool CWeaponBinoculars::Action(s32 cmd, u32 flags) 
{
	switch(cmd) 
	{
	case kWPN_FIRE : 
		return inherited::Action(kWPN_ZOOM, flags);
	}

	return inherited::Action(cmd, flags);
}

void CWeaponBinoculars::OnZoomIn		()
{
	if(H_Parent() && !IsZoomed())
	{
		m_sounds.StopSound("sndZoomOut");
		bool b_hud_mode = (Level().CurrentEntity() == H_Parent());
		m_sounds.PlaySound("sndZoomIn", H_Parent()->Position(), H_Parent(), b_hud_mode);
	}
	inherited::OnZoomIn		();
}

void CWeaponBinoculars::OnZoomOut		()
{
	if(H_Parent() && IsZoomed() && !IsRotatingToZoom())
	{
		m_sounds.StopSound("sndZoomIn");
		bool b_hud_mode = (Level().CurrentEntity() == H_Parent());	
		m_sounds.PlaySound("sndZoomOut", H_Parent()->Position(), H_Parent(), b_hud_mode);
	}


	inherited::OnZoomOut();
}

BOOL CWeaponBinoculars::net_Spawn(CSE_Abstract* DC)
{
	inherited::net_Spawn	(DC);
	return					TRUE;
}

void GetZoomData(const float scope_factor, float& delta, float& min_zoom_factor)
{
	float min_zoom_k = 0.3f;
	float zoom_step_count = 3.0f;
	float delta_factor_total = g_fov - scope_factor;
	VERIFY(delta_factor_total>0);
	min_zoom_factor = g_fov - delta_factor_total*min_zoom_k;
	delta = (delta_factor_total*(1-min_zoom_k) )/zoom_step_count;

}

void CWeaponBinoculars::ZoomInc()
{
	float delta,min_zoom_factor;
	GetZoomData(m_zoom_params.m_fScopeZoomFactor, delta, min_zoom_factor);

	float f					= GetZoomFactor()-delta;
	clamp					(f,m_zoom_params.m_fScopeZoomFactor,min_zoom_factor);
	SetZoomFactor			( f );
}

void CWeaponBinoculars::ZoomDec()
{
	float delta,min_zoom_factor;
	GetZoomData(m_zoom_params.m_fScopeZoomFactor,delta,min_zoom_factor);

	float f					= GetZoomFactor()+delta;
	clamp					(f,m_zoom_params.m_fScopeZoomFactor,min_zoom_factor);
	SetZoomFactor			( f );

}
void CWeaponBinoculars::save(NET_Packet &output_packet)
{
	inherited::save(output_packet);
	save_data		(m_fRTZoomFactor,output_packet);
}

void CWeaponBinoculars::load(IReader &input_packet)
{
	inherited::load(input_packet);
	load_data		(m_fRTZoomFactor,input_packet);
}

void CWeaponBinoculars::GetBriefInfo(xr_string& str_name, xr_string& icon_sect_name, xr_string& str_count)
{
	str_name		= NameShort();
	str_count		= "";
	icon_sect_name	= *cNameSect();
}
