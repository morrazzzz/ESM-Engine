#pragma once

#include "../xr_3da/CustomHUD.h"
#include "HitMarker.h"

class CHUDTarget;
class CUIGameCustom;

class CHUDManager :
	public CCustomHUD
{
	friend class CUI;
private:
    CUIGameCustom*			pUIGame;
	CHitMarker				HitMarker;
	CHUDTarget*				m_pHUDTarget;
	bool					b_online;
public:
							CHUDManager			();
	virtual					~CHUDManager		();
	virtual		void		OnEvent				(EVENT E, u64 P1, u64 P2);

	virtual		void		Load				();
	
	bool NeedRenderHUD(CObject* object) /*override*/;
	void Render_First(CObject* object) override;
	void Render_Last(CObject* object) override;
	virtual		void		OnFrame				();

	virtual		void		RenderUI			();

    CUIGameCustom*		GetGameUI			(){return pUIGame;}

				void		Hit					(int idx, float power, const Fvector& dir);
	//текущий предмет на который смотрит HUD
	collide::rq_result&		GetCurrentRayQuery	();


	//устанвка внешнего вида прицела в зависимости от текущей дисперсии
	void					SetCrosshairDisp	(float dispf, float disps = 0.f);
	void					ShowCrosshair		(bool show);

	void					SetHitmarkType		(LPCSTR tex_name);
	virtual void			OnScreenResolutionChanged();
	virtual void			OnDisconnected		();
	virtual void			OnConnected			();

	virtual void			RenderActiveItemUI	();
	virtual bool			RenderActiveItemUIQuery();


	virtual void			net_Relcase			(CObject *object);
};
