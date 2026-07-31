#include "stdafx.h"
#include "WeaponBinocularsVision.h"
#include "WeaponBinoculars.h"
#include "ui/UIFrameWindow.h"
#include "entity_alive.h"
#include "visual_memory_manager.h"
#include "actor_memory.h"
#include "relation_registry.h"
#include "object_broker.h"

#include "Level.h"
#include "AI/Monsters/BaseMonster/base_monster.h"
#include "../xr_3da/igame_persistent.h"

#define RECT_SIZE 16

extern u32 C_ON_ENEMY;
extern u32 C_ON_NEUTRAL;
extern u32 C_ON_FRIEND;

void SBinocVisibleObj::create_default(u32 color)
{
	Frect r = {0,0,RECT_SIZE,RECT_SIZE};
	m_lt.InitTexture			("ui\\ui_enemy_frame");m_lt.SetWndRect(r);m_lt.SetAlignment(waCenter);
	m_lb.InitTexture			("ui\\ui_enemy_frame");m_lb.SetWndRect(r);m_lb.SetAlignment(waCenter);
	m_rt.InitTexture			("ui\\ui_enemy_frame");m_rt.SetWndRect(r);m_rt.SetAlignment(waCenter);
	m_rb.InitTexture			("ui\\ui_enemy_frame");m_rb.SetWndRect(r);m_rb.SetAlignment(waCenter);

	m_lt.SetTextureRect		(Frect().set(0,				0,				RECT_SIZE,		RECT_SIZE)	);
	m_lb.SetTextureRect		(Frect().set(0,				32-RECT_SIZE,	RECT_SIZE,		32)			);
	m_rt.SetTextureRect		(Frect().set(32-RECT_SIZE,	0,				32,				RECT_SIZE)	);
	m_rb.SetTextureRect		(Frect().set(32-RECT_SIZE,	32-RECT_SIZE,	32,				32)			);


	u32 clr					= subst_alpha(color,128);
	m_lt.SetTextureColor	(clr);
	m_lb.SetTextureColor	(clr);
	m_rt.SetTextureColor	(clr);
	m_rb.SetTextureColor	(clr);

	cur_rect.set			(0,0, UI_BASE_WIDTH,UI_BASE_HEIGHT);

	m_flags.zero			();
}

void SBinocVisibleObj::Draw()
{
	if(m_flags.test(flVisObjNotValid)) return;

	m_lt.Draw			();
	m_lb.Draw			();
	m_rt.Draw			();
	m_rb.Draw			();
}

void SBinocVisibleObj::Update(CObject* object)
{
	m_flags.set		(	flVisObjNotValid,TRUE);


	Fbox b = object->Visual()->getVisData().box;

	Fmatrix				xform;
	xform.mul			(Device.mFullTransform,object->XFORM());
	Fvector2	mn		={flt_max,flt_max},mx={flt_min,flt_min};

	for (u32 k=0; k<8; ++k){
		Fvector p;
		b.getpoint		(k,p);
		xform.transform	(p);
		mn.x			= _min(mn.x,p.x);
		mn.y			= _min(mn.y,p.y);
		mx.x			= _max(mx.x,p.x);
		mx.y			= _max(mx.y,p.y);
	}
	static Frect screen_rect={-1.0f, -1.0f, 1.0f, 1.0f};

	Frect				new_rect;
	new_rect.lt			= mn;
	new_rect.rb			= mx;

	if( FALSE == screen_rect.intersected(new_rect) ) return;
	if( new_rect.in(screen_rect.lt) && new_rect.in(screen_rect.rb) ) return;
	
	std::swap	(mn.y,mx.y);
	mn.x		= (1.f + mn.x)/2.f * UI_BASE_WIDTH;
	mx.x		= (1.f + mx.x)/2.f * UI_BASE_WIDTH;
	mn.y		= (1.f - mn.y)/2.f * UI_BASE_HEIGHT;
	mx.y		= (1.f - mx.y)/2.f * UI_BASE_HEIGHT;

	if (m_flags.is(flTargetLocked)){
		cur_rect.lt.set	(mn);
		cur_rect.rb.set	(mx);
	}else{
		cur_rect.lt.x	+= (mn.x-cur_rect.lt.x)*m_upd_speed*Device.fTimeDelta;
		cur_rect.lt.y	+= (mn.y-cur_rect.lt.y)*m_upd_speed*Device.fTimeDelta;
		cur_rect.rb.x	+= (mx.x-cur_rect.rb.x)*m_upd_speed*Device.fTimeDelta;
		cur_rect.rb.y	+= (mx.y-cur_rect.rb.y)*m_upd_speed*Device.fTimeDelta;
		if (mn.similar(cur_rect.lt,2.f)&&mx.similar(cur_rect.rb,2.f)){ 
			// target locked
			m_flags.set(flTargetLocked,TRUE);
			u32 clr	= subst_alpha(m_lt.GetColor(),255);

			if (Actor())
			{
				//-----------------------------------------------------

				CInventoryOwner* our_inv_owner		= smart_cast<CInventoryOwner*>(Actor());
				CInventoryOwner* others_inv_owner	= smart_cast<CInventoryOwner*>(object);
				CBaseMonster	*monster			= smart_cast<CBaseMonster*>(object);

				if(our_inv_owner && others_inv_owner && !monster){
					switch(RELATION_REGISTRY().GetRelationType(others_inv_owner, our_inv_owner))
					{
					case ALife::eRelationTypeEnemy:
						clr = C_ON_ENEMY; break;
					case ALife::eRelationTypeNeutral:
						clr = C_ON_NEUTRAL; break;
					case ALife::eRelationTypeFriend:
						clr = C_ON_FRIEND; break;
					}
				}
			}

			m_lt.SetColor	(clr);
			m_lb.SetColor	(clr);
			m_rt.SetColor	(clr);
			m_rb.SetColor	(clr);
		}
	}

	m_lt.SetWndPos		( (cur_rect.lt.x)+2,	(cur_rect.lt.y)+2 );
	m_lb.SetWndPos		( (cur_rect.lt.x)+2,	(cur_rect.rb.y)-14 );
	m_rt.SetWndPos		( (cur_rect.rb.x)-14,	(cur_rect.lt.y)+2 );
	m_rb.SetWndPos		( (cur_rect.rb.x)-14,	(cur_rect.rb.y)-14 );

	m_flags.set		(flVisObjNotValid, FALSE);
}


CBinocularsVision::CBinocularsVision(LPCSTR sect)
{
	Load							(sect);
}
CBinocularsVision::~CBinocularsVision()
{
	m_snd_found.destroy	();
	RemoveVisibleObjects();
}

void CBinocularsVision::Update()
{
	PROF_EVENT("Binocular vision update");

	const CActor* pActor = Actor();

	if (!pActor)
		return;
	
	const CVisualMemoryManager::VISIBLES& vVisibles = pActor->memory().visual().objects();
	CVisualMemoryManager::VISIBLES::const_iterator v_it = vVisibles.begin();
	for (; v_it!=vVisibles.end(); ++v_it)
	{
		const CObject*	_object_			= (*v_it).m_object;
		CObject* object_ = const_cast<CObject*>(_object_);
		auto it = m_active_objects.find(object_);
		bool found = it != m_active_objects.end();

		if (!pActor->memory().visual().visible_right_now(static_cast<const CGameObject*>(object_)))
		{
			if (found)
				it->second->m_flags.set(flVisObjNotValid, true);

			continue;
		}

		CEntityAlive*	EA = smart_cast<CEntityAlive*>(object_);
		if (!EA || !EA->g_Alive())
		{
			if (found)
				it->second->m_flags.set(flVisObjNotValid, true);

			continue;
		}

		//found = std::find_if				(m_active_objects.begin(),m_active_objects.end(),f);

		if (!found)
		{
			SBinocVisibleObj* new_vis_obj = new SBinocVisibleObj();
			new_vis_obj->m_flags.set(flVisObjNotValid, FALSE);
			new_vis_obj->create_default(m_frame_color.get());
			new_vis_obj->m_upd_speed = m_rotating_speed;
			if (NULL == m_snd_found._feedback())
				m_snd_found.play_at_pos(0, Fvector().set(0, 0, 0), sm_2D);

			new_vis_obj->Update(object_);

			m_active_objects[object_] = new_vis_obj;
		}
		else
			it->second->Update(it->first);
	}

	for (auto a_it = m_active_objects.begin(); a_it != m_active_objects.end();)
	{
		if (a_it->second->m_flags.test(flVisObjNotValid))
		{
			delete a_it->second;
			m_active_objects.erase(a_it++);
		}
		else
			a_it++;
	}
}

void CBinocularsVision::Draw()
{
	for (const auto& it : m_active_objects)
		it.second->Draw();
}

void CBinocularsVision::Load(LPCSTR section)
{
	m_rotating_speed	= pSettings->r_float(section,"vis_frame_speed");
	m_frame_color		= pSettings->r_fcolor(section,"vis_frame_color");
	m_snd_found.create	(pSettings->r_string(section,"found_snd"),st_Effect,sg_SourceType);
}

void CBinocularsVision::remove_links(CObject *object)
{
	auto it = m_active_objects.find(object);
	if (it == m_active_objects.end())
		return;

	m_active_objects.erase(it);
}
