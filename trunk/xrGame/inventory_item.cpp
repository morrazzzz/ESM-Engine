////////////////////////////////////////////////////////////////////////////
//	Module 		: inventory_item.cpp
//	Created 	: 24.03.2003
//  Modified 	: 29.01.2004
//	Author		: Victor Reutsky, Yuri Dobronravin
//	Description : Inventory item
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "inventory_item.h"
#include "inventory_item_impl.h"
#include "inventory.h"
#include "xrserver_objects_alife.h"
#include "xrserver_objects_alife_items.h"
#include "entity_alive.h"
#include "Level.h"
#include "Actor.h"
#include "string_table.h"
#include "..\include\xrRender\Kinematics.h"
#include "ai_object_location.h"
#include "object_broker.h"
#include "../xr_3da/igame_persistent.h"
#include "../xrPhysics/IPHWorld.h"

#ifdef DEBUG
#include "level_debug.h"
#include "debug_renderer.h"
#endif

#define ITEM_REMOVE_TIME		30000
struct net_update_IItem {	u32					dwTimeStamp;
SPHNetState			State;};
struct net_updateData{



	xr_deque<net_update_IItem>	NET_IItem;
	/// spline coeff /////////////////////
	float			SCoeff[3][4];

#ifdef DEBUG
	DEF_VECTOR		(VIS_POSITION, Fvector);
	VIS_POSITION	LastVisPos;
#endif

	Fvector			IStartPos;
	Fquaternion		IStartRot;

	Fvector			IRecPos;
	Fquaternion		IRecRot;

	Fvector			IEndPos;
	Fquaternion		IEndRot;	

	SPHNetState		LastState;
	SPHNetState		RecalculatedState;

#ifdef DEBUG
	SPHNetState		CheckState;
#endif
	SPHNetState		PredictedState;

	u32				m_dwIStartTime;
	u32				m_dwIEndTime;
	u32				m_dwILastUpdateTime;
};

net_updateData* CInventoryItem::NetSync()			
{
	if(!m_net_updateData) 
		m_net_updateData = xr_new<net_updateData>();
	return m_net_updateData;
}

CInventoryItem::CInventoryItem() 
{
	m_net_updateData	= NULL;
	m_slot				= NO_ACTIVE_SLOT;
	m_flags.set			(Fbelt,FALSE);
	m_flags.set			(Fruck,TRUE);
	m_flags.set			(FRuckDefault,TRUE);
	m_pCurrentInventory	= NULL;

	SetDroppedItem(false);

	m_flags.set			(FCanTake,TRUE);
	m_flags.set			(FCanTrade,TRUE);
	m_flags.set			(FUsingCondition,FALSE);
	m_fCondition		= 1.0f;

	m_name = m_nameShort = NULL;

	m_eItemPlace		= eItemPlaceUndefined;
	m_Description		= "";
}

CInventoryItem::~CInventoryItem() 
{
	delete_data			(m_net_updateData);

	bool B_GOOD			= (	!m_pCurrentInventory || 
							(std::find(	m_pCurrentInventory->m_all.begin(),m_pCurrentInventory->m_all.end(), this)==m_pCurrentInventory->m_all.end()) );
	if(!B_GOOD)
	{
		CObject* p	= object().H_Parent();
		Msg("inventory ptr is [%s]",m_pCurrentInventory?"not-null":"null");
		if(p)
			Msg("parent name is [%s]",p->cName().c_str());

			Msg("! ERROR item_id[%d] H_Parent=[%s][%d] [%d]",
				object().ID(),
				p ? p->cName().c_str() : "none",
				p ? p->ID() : -1,
				Device.dwFrame);
	}
}

void CInventoryItem::Load(LPCSTR section) 
{
	CHitImmunity::LoadImmunities	(pSettings->r_string(section,"immunities_sect"),pSettings);

	ISpatial*			self				=	smart_cast<ISpatial*> (this);
	if (self)			self->spatial.type	|=	STYPE_VISIBLEFORAI;	

	m_name				= CStringTable().translate( pSettings->r_string(section, "inv_name") );
	m_nameShort			= CStringTable().translate( pSettings->r_string(section, "inv_name_short"));

//.	NameComplex			();
	m_weight			= pSettings->r_float(section, "inv_weight");
	R_ASSERT			(m_weight>=0.f);

	m_cost				= pSettings->r_u32(section, "cost");

	m_slot				= READ_IF_EXISTS(pSettings,r_u32,section,"slot", NO_ACTIVE_SLOT);


	// Description
	if ( pSettings->line_exist(section, "description") )
		m_Description = CStringTable().translate( pSettings->r_string(section, "description") );

	m_flags.set(Fbelt,			READ_IF_EXISTS(pSettings, r_bool, section, "belt",				FALSE));
	m_flags.set(FRuckDefault,	READ_IF_EXISTS(pSettings, r_bool, section, "default_to_ruck",	TRUE));

	m_flags.set(FCanTake,		READ_IF_EXISTS(pSettings, r_bool, section, "can_take",			TRUE));
	m_flags.set(FCanTrade,		READ_IF_EXISTS(pSettings, r_bool, section, "can_trade",			TRUE));
	m_flags.set(FIsQuestItem,	READ_IF_EXISTS(pSettings, r_bool, section, "quest_item",		FALSE));



	//время убирания объекта с уровня
	m_dwItemRemoveTime			= READ_IF_EXISTS(pSettings, r_u32, section,"item_remove_time",			ITEM_REMOVE_TIME);

	m_flags.set					(FAllowSprint,READ_IF_EXISTS	(pSettings, r_bool, section,"sprint_allowed",			TRUE));
	m_fControlInertionFactor	= READ_IF_EXISTS(pSettings, r_float,section,"control_inertion_factor",	1.0f);
	m_icon_name					= READ_IF_EXISTS(pSettings, r_string,section,"icon_name",				NULL);

}


void  CInventoryItem::ChangeCondition(float fDeltaCondition)
{
	m_fCondition += fDeltaCondition;
	clamp(m_fCondition, 0.f, 1.f);
}


void	CInventoryItem::Hit					(SHit* pHDS)
{
	if( !m_flags.test(FUsingCondition) ) return;

	float hit_power = pHDS->damage();
	hit_power *= m_HitTypeK[pHDS->hit_type];

	ChangeCondition(-hit_power);
}

const char* CInventoryItem::Name() 
{
	return *m_name;
}

const char* CInventoryItem::NameShort() 
{
	return *m_nameShort;
}

bool CInventoryItem::Useful() const
{
	return CanTake();
}

bool CInventoryItem::Activate() 
{
	return false;
}

void CInventoryItem::Deactivate() 
{
}

void CInventoryItem::OnH_B_Independent(bool just_before_destroy)
{
	UpdateXForm();
	m_eItemPlace = eItemPlaceUndefined ;
}

void CInventoryItem::OnH_A_Independent()
{
	m_dwItemIndependencyTime	= Level().timeServer();
	m_eItemPlace				= eItemPlaceUndefined;	
	inherited::OnH_A_Independent();
}

void CInventoryItem::OnH_B_Chield()
{
}

void CInventoryItem::OnH_A_Chield()
{
	inherited::OnH_A_Chield		();
}

void CInventoryItem::UpdateCL()
{
#ifdef DEBUG
	if(bDebug){
		if (dbg_net_Draw_Flags.test(dbg_draw_invitem))
		{
			Device.seqRender.Remove(this);
			Device.seqRender.Add(this);
		}else
		{
			Device.seqRender.Remove(this);
		}
	}

#endif

}

void CInventoryItem::OnEvent (NET_Packet& P, u16 type)
{
}

//процесс отсоединения вещи заключается в спауне новой вещи 
//в инвентаре и установке соответствующих флагов в родительском
//объекте, поэтому функция должна быть переопределена
bool CInventoryItem::Detach(const char* item_section_name, bool b_spawn_item) 
{
	if(b_spawn_item)
	{
		CSE_Abstract*		D	= F_entity_Create(item_section_name);
		R_ASSERT		   (D);
		CSE_ALifeDynamicObject	*l_tpALifeDynamicObject = 
								smart_cast<CSE_ALifeDynamicObject*>(D);
		R_ASSERT			(l_tpALifeDynamicObject);
		
		l_tpALifeDynamicObject->m_tNodeID = (g_dedicated_server)?u32(-1):object().ai_location().level_vertex_id();
			
		// Fill
		D->s_name			=	item_section_name;
		D->set_name_replace	("");
		D->s_gameid			=	u8(GameID());
		D->s_RP				=	0xff;
		D->ID				=	0xffff;
		if (GameID() == GAME_SINGLE)
		{
			D->ID_Parent		=	u16(object().H_Parent()->ID());
		}
		else	// i'm not sure this is right
		{		// but it is simpliest way to avoid exception in MP BuyWnd... [Satan]
			if (object().H_Parent())
				D->ID_Parent	=	u16(object().H_Parent()->ID());
			else
				D->ID_Parent	= NULL;
		}
		D->ID_Phantom		=	0xffff;
		D->o_Position		=	object().Position();
		D->s_flags.assign	(M_SPAWN_OBJECT_LOCAL);
		D->RespawnTime		=	0;
		// Send
		NET_Packet			P;
		D->Spawn_Write		(P,TRUE);
		Level().Send		(P);
		// Destroy
		F_entity_Destroy	(D);
	}
	return true;
}

/////////// network ///////////////////////////////
BOOL CInventoryItem::net_Spawn			(CSE_Abstract* DC)
{
	m_flags.set						(FInInterpolation, FALSE);
	m_flags.set						(FInInterpolate,	FALSE);
//	m_bInInterpolation				= false;
//	m_bInterpolate					= false;

	m_flags.set						(Fuseful_for_NPC, TRUE);
	CSE_Abstract					*e	= (CSE_Abstract*)(DC);
	CSE_ALifeObject					*alife_object = smart_cast<CSE_ALifeObject*>(e);
	if (alife_object)	{
		m_flags.set(Fuseful_for_NPC, alife_object->m_flags.test(CSE_ALifeObject::flUsefulForAI));
	}

	CSE_ALifeInventoryItem			*pSE_InventoryItem = smart_cast<CSE_ALifeInventoryItem*>(e);
	if (!pSE_InventoryItem)			return TRUE;

	//!!!
	m_fCondition = pSE_InventoryItem->m_fCondition;

	m_dwItemIndependencyTime		= 0;

	return							TRUE;
}

void CInventoryItem::net_Destroy		()
{
	//инвентарь которому мы принадлежали
//.	m_pCurrentInventory = NULL;
}

void CInventoryItem::save(NET_Packet &packet)
{
	packet.w_u8				((u8)m_eItemPlace);
	packet.w_float			(m_fCondition);

	if (object().H_Parent()) {
		packet.w_u8			(0);
		return;
	}

	u8 _num_items			= (u8)object().PHGetSyncItemsNumber(); 
	packet.w_u8				(_num_items);
	object().PHSaveState	(packet);
}

typedef CSE_ALifeInventoryItem::mask_num_items	mask_num_items;

void CInventoryItem::load(IReader &packet)
{
	m_eItemPlace			= (EItemPlace)packet.r_u8();
	m_fCondition			= packet.r_float();

	u8						tmp = packet.r_u8();
	if (!tmp)
		return;
	
	if (!object().PPhysicsShell()) {
		object().setup_physic_shell	();
		object().PPhysicsShell()->Disable();
	}
	
	object().PHLoadState(packet);
	object().PPhysicsShell()->Disable();
}

void CInventoryItem::reload		(LPCSTR section)
{
	inherited::reload		(section);
	m_holder_range_modifier	= READ_IF_EXISTS(pSettings,r_float,section,"holder_range_modifier",1.f);
	m_holder_fov_modifier	= READ_IF_EXISTS(pSettings,r_float,section,"holder_fov_modifier",1.f);
}

void CInventoryItem::reinit		()
{
	m_pCurrentInventory	= NULL;
	m_eItemPlace	= eItemPlaceUndefined;
}

bool CInventoryItem::can_kill			() const
{
	return				(false);
}

CInventoryItem *CInventoryItem::can_kill	(CInventory *inventory) const
{
	return				(0);
}

const CInventoryItem *CInventoryItem::can_kill			(const xr_vector<const CGameObject*> &items) const
{
	return				(0);
}

CInventoryItem *CInventoryItem::can_make_killing	(const CInventory *inventory) const
{
	return				(0);
}

bool CInventoryItem::ready_to_kill		() const
{
	return				(false);
}

void CInventoryItem::activate_physic_shell()
{
	CEntityAlive*	E		= smart_cast<CEntityAlive*>(object().H_Parent());
	if (!E) {
		on_activate_physic_shell();
		return;
	};

	UpdateXForm();

	object().CPhysicsShellHolder::activate_physic_shell();
}

void CInventoryItem::UpdateXForm	()
{
	if (0==object().H_Parent())	return;

	// Get access to entity and its visual
	CEntityAlive*	E		= smart_cast<CEntityAlive*>(object().H_Parent());
	if (!E) return;
	
	if (E->cast_base_monster()) return;

	const CInventoryOwner	*parent = smart_cast<const CInventoryOwner*>(E);
	if (parent && parent->use_simplified_visual())
		return;

	if (parent->attached(this))
		return;

	R_ASSERT		(E);
	IKinematics*	V		= smart_cast<IKinematics*>	(E->Visual());
	VERIFY			(V);

	// Get matrices
	int				boneL,boneR,boneR2;
	E->g_WeaponBones(boneL,boneR,boneR2);
	//	if ((HandDependence() == hd1Hand) || (STATE == eReload) || (!E->g_Alive()))
	//		boneL = boneR2;
#pragma todo("TO ALL: serious performance problem")
	V->CalculateBones	();
	Fmatrix& mL			= V->LL_GetTransform(u16(boneL));
	Fmatrix& mR			= V->LL_GetTransform(u16(boneR));
	// Calculate
	Fmatrix			mRes;
	Fvector			R,D,N;
	D.sub			(mL.c,mR.c);	D.normalize_safe();

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

	//	UpdatePosition	(mRes);
	object().Position().set(mRes.c);
}

void CInventoryItem::DropItem()
{
	SetDroppedItem(true);

	CGameObject* object_parent = static_cast<CGameObject*>(object().H_Parent());

	object_parent->RejectItem(cast_game_object());
}

#ifdef DEBUG

void CInventoryItem::OnRender()
{
	if (bDebug && object().Visual())
	{
		if (!(dbg_net_Draw_Flags.is_any(dbg_draw_invitem)))
			return;

		Fvector bc,bd; 
		object().Visual()->getVisData().box.get_CD(bc, bd);
		Fmatrix	M = object().XFORM();
		M.c.add (bc);
		Level().debug_renderer().draw_obb			(M,bd,color_rgba(0,0,255,255));
	};
}
#endif

DLL_Pure *CInventoryItem::_construct	()
{
	m_object	= smart_cast<CPhysicsShellHolder*>(this);
	VERIFY		(m_object);
	return		(inherited::_construct());
}

void CInventoryItem::modify_holder_params	(float &range, float &fov) const
{
	range		*= m_holder_range_modifier;
	fov			*= m_holder_fov_modifier;
}

ALife::_TIME_ID	 CInventoryItem::TimePassedAfterIndependant()	const
{
	if(!object().H_Parent() && m_dwItemIndependencyTime != 0)
		return Level().timeServer() - m_dwItemIndependencyTime;
	else
		return 0;
}

bool	CInventoryItem::CanTrade() const 
{
	bool res = true;
#pragma todo("Dima to Andy : why CInventoryItem::CanTrade can be called for the item, which doesn't have owner?")
	if(m_pCurrentInventory)
		res = inventory_owner().AllowItemToTrade(this,m_eItemPlace);

	return (res && m_flags.test(FCanTrade) && !IsQuestItem());
}

int  CInventoryItem::GetGridWidth			() const 
{
	return pSettings->r_u32(m_object->cNameSect(), "inv_grid_width");
}

int  CInventoryItem::GetGridHeight			() const 
{
	return pSettings->r_u32(m_object->cNameSect(), "inv_grid_height");
}
int  CInventoryItem::GetXPos				() const 
{
	return pSettings->r_u32(m_object->cNameSect(), "inv_grid_x");
}
int  CInventoryItem::GetYPos				() const 
{
	return pSettings->r_u32(m_object->cNameSect(), "inv_grid_y");
}

bool CInventoryItem::IsNecessaryItem(CInventoryItem* item)		
{
	return IsNecessaryItem(item->object().cNameSect());
};

bool CInventoryItem::IsInvalid() const
{
	return object().getDestroy() || GetDroppedItem();
}

u16 CInventoryItem::bone_count_to_synchronize	() const
{
	return 0;
}