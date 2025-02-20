#pragma once
#include "ui\uistatic.h"
class CObject;

enum{
	flVisObjNotValid		=(1<<0),
	flTargetLocked			=(1<<1),
};
struct SBinocVisibleObj{
	SBinocVisibleObj() {};
	CUIStatic				m_lt;
	CUIStatic				m_lb;
	CUIStatic				m_rt;
	CUIStatic				m_rb;
	Frect					cur_rect;

	float					m_upd_speed;
	Flags8					m_flags;
	void create_default(u32 color);
	void Draw();
	void Update(CObject*);
};

class CBinocularsVision
{
	xr_unordered_map<CObject*, SBinocVisibleObj*> m_active_objects;
public:
	CBinocularsVision			(LPCSTR sect);
	~CBinocularsVision			();
	void	Update				();
	void	Draw				();
	void	remove_links		(CObject *object);
	IC void RemoveVisibleObjects() 
	{
		for (auto& it : m_active_objects)
			delete it.second;

		m_active_objects.clear();
	};

protected :
	Fcolor						m_frame_color;
	float						m_rotating_speed;
	void	Load				(LPCSTR section);
	ref_sound					m_snd_found;
};