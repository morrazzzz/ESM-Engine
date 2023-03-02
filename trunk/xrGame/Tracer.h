// Tracer.h: interface for the CTracer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CBulletManager;

class CTracer
{
	friend CBulletManager;
protected:
	ref_shader			sh_Tracer;
	ref_geom			sh_Geom;
	xr_vector<u32>		m_aColors;			
public:
						CTracer		();
						~CTracer	();

	void				Render		(FVF::LIT*&verts, const Fvector& pos, const Fvector& center, const Fvector& dir, float length, float width, u8 colorID);
};
