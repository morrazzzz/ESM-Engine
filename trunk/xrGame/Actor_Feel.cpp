#include "stdafx.h"
#include "actor.h"
#include "weapon.h"
#include "mercuryball.h"
#include "inventory.h"
#include "hudmanager.h"
#include "UsableScriptObject.h"
#include "customzone.h"
#include "../xr_3da/GameMtlLib.h"
#include "ui/UIMainIngameWnd.h"
#include "UIGameCustom.h"
#include "Grenade.h"
#include "clsid_game.h"

#include "game_cl_base.h"
#include "Level.h"

#define PICKUP_INFO_COLOR 0xFFDDDDDD
//AAAAAA

void CActor::feel_touch_new				(CObject* O)
{
}

void CActor::feel_touch_delete	(CObject* O)
{
	CPhysicsShellHolder* sh=smart_cast<CPhysicsShellHolder*>(O);
	if(sh&&sh->character_physics_support()) m_feel_touch_characters--;
}

BOOL CActor::feel_touch_contact		(CObject *O)
{
	CInventoryItem	*item = smart_cast<CInventoryItem*>(O);
	CInventoryOwner	*inventory_owner = smart_cast<CInventoryOwner*>(O);

	if (item && item->Useful() && !item->object().H_Parent()) 
		return TRUE;

	if(inventory_owner && inventory_owner != smart_cast<CInventoryOwner*>(this))
	{
		CPhysicsShellHolder* sh=smart_cast<CPhysicsShellHolder*>(O);
		if(sh&&sh->character_physics_support()) m_feel_touch_characters++;
		return TRUE;
	}

	return		(FALSE);
}

BOOL CActor::feel_touch_on_contact	(CObject *O)
{
	CCustomZone	*custom_zone = smart_cast<CCustomZone*>(O);
	if (!custom_zone)
		return	(TRUE);

	Fsphere		sphere;
	sphere.P	= Position();
	sphere.R	= EPS_L;
	if (custom_zone->inside(sphere))
		return	(TRUE);

	return		(FALSE);
}

void CActor::PickupModeOn()
{
	m_bPickupMode = true;
}

void CActor::PickupModeOff()
{
	m_bPickupMode = false;
}

ICF static bool info_trace_callback(collide::rq_result& result, LPVOID params)
{
	if (result.O) {
		if (Level().CurrentEntity() != result.O) {
			//			bOverlaped		= TRUE;
			return true;//FALSE;
		}
		else {
			return true;
		}
	}
	else {
		//�������� ����������� � ������ ��� ��������
		CDB::TRI* T = Level().ObjectSpace.GetStaticTris() + result.element;
		if (GMLib.GetMaterialByIdx(T->material)->Flags.is(SGameMtl::flPassable))
			return true;
	}
	return false;
}

bool CActor::CanPickItem(const CFrustum& frustum, const Fvector& from, CObject* item)
{
	bool bOverlaped = false;
	Fvector dir, to;
	item->Center(to);
	float range = dir.sub(to, from).magnitude();
	if (range > 0.25f) {
		if (frustum.testSphere_dirty(to, item->Radius())) {
			dir.div(range);
			collide::ray_defs			RD(from, dir, range, CDB::OPT_CULL, collide::rqtBoth);
			VERIFY(!fis_zero(RD.dir.square_magnitude()));
			RQR.r_clear();
			Level().ObjectSpace.RayQuery(RQR, RD, info_trace_callback, &bOverlaped, NULL, item);
		}
	}
	return !bOverlaped;
}

void CActor::PickupModeUpdateAll()
{
	if (g_Alive())
	{
		PickupModeUpdate();

		CFrustum frustum;
		frustum.CreateFromMatrix(Device.mFullTransform_saved, FRUSTUM_P_LRTB | FRUSTUM_P_FAR);

		PickupModeUpdate_COD(frustum);

		if (m_bPickupMode)
		{
			//. ????? GetNearest ?????
			feel_touch_update(Position(), /*inventory().GetTakeDist()*/m_fPickupInfoRadius);

			//. slow (ray-query test)
			for (u32 i = 0; i < feel_touch.size(); i++)
			{
				if (auto item = smart_cast<CInventoryItem*>(feel_touch[i]))
				{
					if (CanPickItem(frustum, Device.vCameraPosition_saved, feel_touch[i]))
						PickupInfoDraw(item->Name(), feel_touch[i]->XFORM());
				}
			}
		}
	}
}

void CActor::PickupModeUpdate()
{
	if(!m_bPickupMode) 
		return;

//	if (inventory().m_pTarget && inventory().m_pTarget->object().getDestroy())
//		return;
		
	//���������� �������
	if(inventory().m_pTarget && inventory().m_pTarget->Useful() &&
		m_pUsableObject && m_pUsableObject->nonscript_usable() &&
		!Level().m_feel_deny.is_object_denied(smart_cast<CGameObject*>(inventory().m_pTarget)) )
	{
		NET_Packet P;
		u_EventGen(P,GE_OWNERSHIP_TAKE, ID());
		P.w_u16(inventory().m_pTarget->object().ID());
		u_EventSend(P);
	}
}

#include "../xr_3da/CameraBase.h"
bool g_b_COD_PickUpMode = true;
void CActor::PickupModeUpdate_COD(const CFrustum& frustum)
{
	if (Level().CurrentViewEntity() != this || !g_b_COD_PickUpMode) return;
		
	if (!g_Alive() || eacFirstEye != cam_active) 
	{
		CurrentGameUI()->UIMainIngameWnd->SetPickUpItem(NULL);
		return;
	};

	//---------------------------------------------------------------------------
	ISpatialResultPickup.clear_not_free	();
	g_SpatialSpace->q_frustum(ISpatialResultPickup, 0, STYPE_COLLIDEABLE, frustum);
	//---------------------------------------------------------------------------

	float maxlen = 1000.0f;
	CInventoryItem* pNearestItem = NULL;
	for (u32 o_it=0; o_it< ISpatialResultPickup.size(); o_it++)
	{
		ISpatial*		spatial	= ISpatialResultPickup[o_it];
		CInventoryItem*	pIItem	= smart_cast<CInventoryItem*> (spatial->dcast_CObject        ());
		if (0 == pIItem) continue;
		if (pIItem->object().H_Parent() != NULL) continue;
		if (!pIItem->CanTake()) continue;
		if (pIItem->object().CLS_ID == CLSID_OBJECT_G_RPG7 || pIItem->object().CLS_ID == CLSID_OBJECT_G_FAKE)
			continue;

		CGrenade*	pGrenade	= smart_cast<CGrenade*> (spatial->dcast_CObject        ());
		if (pGrenade && !pGrenade->Useful()) continue;

		CMissile*	pMissile	= smart_cast<CMissile*> (spatial->dcast_CObject        ());
		if (pMissile && !pMissile->Useful()) continue;
		
		Fvector A, B, tmp; 
		pIItem->object().Center			(A);
		if (A.distance_to_sqr(Position())>4) continue;

		tmp.sub(A, cam_Active()->vPosition);
		B.mad(cam_Active()->vPosition, cam_Active()->vDirection, tmp.dotproduct(cam_Active()->vDirection));
		float len = B.distance_to_sqr(A);
		if (len > 1) continue;

		if (maxlen>len && !pIItem->object().getDestroy())
		{
			maxlen = len;
			pNearestItem = pIItem;
		};
	}

	if(pNearestItem)
	{
		if (!CanPickItem(frustum,Device.vCameraPosition_saved, &pNearestItem->object()))
			pNearestItem = NULL;
	}

	if (pNearestItem && pNearestItem->cast_game_object())
	{
		if (Level().m_feel_deny.is_object_denied(pNearestItem->cast_game_object()))
				pNearestItem = NULL;
	}

	CurrentGameUI()->UIMainIngameWnd->SetPickUpItem(pNearestItem);
	

	if (pNearestItem && m_bPickupMode)
	{
		//���������� �������
		Game().SendPickUpEvent(ID(), pNearestItem->object().ID());
		
		PickupModeOff();
	}
};

void CActor::PickupInfoDraw(LPCSTR item_name, const Fmatrix& item_xform)
{
	Fmatrix	res;
	res.mul(Device.mFullTransform_saved, item_xform);
	Fvector4 v_res;
	Fvector	shift;

	shift.set(0, 0, 0);

	res.transform(v_res, shift);

	if (v_res.z < 0 || v_res.w < 0)	return;
	if (v_res.x < -1.f || v_res.x > 1.f || v_res.y < -1.f || v_res.y>1.f) return;

	float x = (1.f + v_res.x) / 2.f * (Device.dwWidth);
	float y = (1.f - v_res.y) / 2.f * (Device.dwHeight);

	std::lock_guard lock(PickipModeMutex);
	UI().Font().pFontLetterica16Russian->SetAligment(CGameFont::alCenter);
	UI().Font().pFontLetterica16Russian->SetColor(PICKUP_INFO_COLOR);
	UI().Font().pFontLetterica16Russian->Out(x, y, item_name);
}

void CActor::feel_sound_new(CObject* who, int type, CSound_UserDataPtr user_data, const Fvector& Position, float power)
{
	if(who == this)
		m_snd_noise = _max(m_snd_noise,power);
}
