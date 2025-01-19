#pragma once

#include "iinputreceiver.h"

ENGINE_API extern Flags32		psHUD_Flags;
#define HUD_CROSSHAIR			(1<<0)
#define HUD_CROSSHAIR_DIST		(1<<1)
#define HUD_WEAPON				(1<<2)
#define HUD_INFO				(1<<3)
#define HUD_DRAW				(1<<4)
#define HUD_CROSSHAIR_RT		(1<<5)
#define HUD_WEAPON_RT			(1<<6)
#define HUD_CROSSHAIR_DYNAMIC	(1<<7)
#define HUD_CROSSHAIR_RT2		(1<<9)
#define HUD_DRAW_RT				(1<<10)

class CUI;

class ENGINE_API CCustomHUD:
	public DLL_Pure,
	public IEventReceiver,
	public pureScreenResolutionChanged
{
public:
					CCustomHUD				();
	virtual			~CCustomHUD				();

	//virtual bool NeedRenderHUD(CObject* object) = 0;
	virtual		void		Render_First (CObject* object){;}
	virtual		void		Render_Last	(CObject* object){;}
	
	virtual		void		OnFrame					(){;}
	virtual		void		OnEvent					(EVENT E, u64 P1, u64 P2){;}

	virtual		void		Load					(){;}
	virtual void			OnDisconnected			()=0;
	virtual void			OnConnected				()=0;
	virtual void            RenderActiveItemUI      ()=0;
	virtual bool            RenderActiveItemUIQuery ()=0;
	virtual void			net_Relcase				(CObject *object) = 0;
};

extern ENGINE_API CCustomHUD* g_hud;