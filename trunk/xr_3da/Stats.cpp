#include "stdafx.h"
#include "GameFont.h"
#include "IGame_Persistent.h"
#include "render.h"
#include "xr_object.h"

#include "../xrCDB/ISpatial.h"
#include "../xrCDB/xrCDB_Measuring.h"

#include "../Include/xrRender/DrawUtils.h"

int		g_ErrorLineCount	= 15;
Flags32 g_stats_flags		= {0};

extern BOOL DisplayEngineInformation_;
extern BOOL DisplayFPSShow_;

// stats
DECLARE_RP(Stats);

class	optimizer	{
	float	average_	;
	BOOL	enabled_	;
public:
	optimizer	()		{
		average_	= 30.f;
//		enabled_	= TRUE;
//		disable		();
		// because Engine is not exist
		enabled_	= FALSE;
	}

	BOOL	enabled	()	{ return enabled_;	}
	void	enable	()	{ if (!enabled_)	{ Engine.External.tune_resume	();	enabled_=TRUE;	}}
	void	disable	()	{ if (enabled_)		{ Engine.External.tune_pause	();	enabled_=FALSE; }}
	void	update	(float value)	{
		if (value < average_*0.7f)	{
			// 25% deviation
			enable	();
		} else {
			disable	();
		};
		average_	= 0.99f*average_ + 0.01f*value;
	};
};
static	optimizer	vtune;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
BOOL			g_bDisableRedText	= FALSE;
CStats::CStats	()
{
	fFPS				= 30.f;
	fRFPS				= 30.f;
	fTPS				= 0;
	pFont				= 0;
	fMem_calls			= 0;
	RenderDUMP_DT_Count = 0;
	Device.seqRender.Add		(this,REG_PRIORITY_LOW-1000);
}

CStats::~CStats()
{
	Device.seqRender.Remove		(this);
	xr_delete		(pFont);
}

void _draw_cam_pos(CGameFont* pFont)
{
	float sz		= pFont->GetHeight();
	pFont->SetHeightI(0.02f);
	pFont->SetColor(color_rgba(255, 255, 0, 250));
	pFont->Out(10, 230, "CAMERA POSITION:  [%3.2f,%3.2f,%3.2f]", VPUSH(Device.vCameraPosition));
	pFont->SetHeight(sz);
	pFont->OnRender();
}
void draw_fps(CGameFont* pFont)
{
	float sz = pFont->GetHeight();
	pFont->SetHeightI(0.025f);
	pFont->SetColor(color_rgba(0, 255, 0, 250));
	pFont->Out(720, 10, "FPS: %1.0f", 1.0f / Device.fTimeDelta);
	pFont->SetHeight(sz);
	pFont->OnRender	();
}

void CStats::Show() 
{
	// Stop timers
	{
		EngineTOTAL.FrameEnd		();	
		Sheduler.FrameEnd			();	
		UpdateClient.FrameEnd		();	
		Physics.FrameEnd			();	
		ph_collision.FrameEnd		();
		ph_core.FrameEnd			();
		Animation.FrameEnd			();	
		AI_Think.FrameEnd			();
		AI_Range.FrameEnd			();
		AI_Path.FrameEnd			();
		AI_Node.FrameEnd			();
		AI_Vis.FrameEnd				();
		AI_Vis_Query.FrameEnd		();
		AI_Vis_RayTests.FrameEnd	();
		
		RenderTOTAL.FrameEnd		();
		RenderCALC.FrameEnd			();
		RenderCALC_HOM.FrameEnd		();
		RenderDUMP.FrameEnd			();	
		RenderDUMP_RT.FrameEnd		();
		RenderDUMP_SKIN.FrameEnd	();	
		RenderDUMP_Wait.FrameEnd	();	
		RenderDUMP_Wait_S.FrameEnd	();	
		RenderDUMP_HUD.FrameEnd		();	
		RenderDUMP_Rain.FrameEnd    ();
		RenderDUMP_Glows.FrameEnd	();	
		RenderDUMP_Lights.FrameEnd	();	
		RenderDUMP_WM.FrameEnd		();	
		RenderDUMP_DT_VIS.FrameEnd	();	
		RenderDUMP_DT_Render.FrameEnd();	
		RenderDUMP_DT_Cache.FrameEnd();
		RenderDUMP_Pcalc.FrameEnd	();	
		RenderDUMP_Scalc.FrameEnd	();	
		RenderDUMP_Srender.FrameEnd	();	
		
		Sound.FrameEnd				();
		Input.FrameEnd				();
		
		CDB_RAY_TIMER(FrameEnd());
		CDB_BOX_TIMER(FrameEnd());
		CDB_FRUSTUM_TIMER(FrameEnd());
		
		netClient1.FrameEnd			();
		netClient2.FrameEnd			();
		netServer.FrameEnd			();
		
		netClientCompressor.FrameEnd();
		netServerCompressor.FrameEnd();

		BulletManager.FrameEnd();
		ActorCameraUpdate.FrameEnd();
		CreateListNetExport.FrameEnd();
		SendNetExport.FrameEnd				();

		g_SpatialSpace->stat_insert.FrameEnd		();
		g_SpatialSpace->stat_remove.FrameEnd		();
		g_SpatialSpacePhysic->stat_insert.FrameEnd	();
		g_SpatialSpacePhysic->stat_remove.FrameEnd	();
	}

	// calc FPS & TPS
	if (Device.fTimeDelta>EPS_S) {
		float fps  = 1.f/Device.fTimeDelta;
		//if (Engine.External.tune_enabled)	vtune.update	(fps);
		float fOne = 0.3f;
		float fInv = 1.f-fOne;
		fFPS = fInv*fFPS + fOne*fps;

		if (RenderTOTAL.result>EPS_S) {
			u32	rendered_polies = Device.m_pRender->GetCacheStatPolys();
			fTPS = fInv * fTPS + fOne * float(rendered_polies) / (RenderTOTAL.result * 1000.f);
			fRFPS= fInv*fRFPS+ fOne*1000.f/RenderTOTAL.result;
		}
	}
	{
		float mem_count		= float	(Memory.stat_calls);
		if (mem_count>fMem_calls)	fMem_calls	=	mem_count;
		else						fMem_calls	=	.9f*fMem_calls + .1f*mem_count;
		Memory.stat_calls	= 0		;
	}

    if (psDeviceFlags.test(rsDrawMemory))
	{
		float sz = pFont->GetHeight();
		pFont->SetHeightI(0.02f);
		pFont->SetColor(color_rgba(0, 255, 0, 250));
		pFont->Out(720, 30, "Memory: %3.0f", fMem_calls);
		pFont->SetHeight(sz);
		pFont->OnRender();
	}
	CGameFont& F = *pFont;
	float		f_base_size	= 0.01f;
				F.SetHeightI	(f_base_size);

	if (vtune.enabled())	{
		float sz		= pFont->GetHeight();
		pFont->SetHeightI(0.02f);
		pFont->SetColor	(0xFFFF0000	);
		pFont->OutSet	(Device.dwWidth/2.0f+(pFont->SizeOf_("--= tune =--")/2.0f),Device.dwHeight/2.0f);
		pFont->OutNext	("--= tune =--");
		pFont->OnRender	();
		pFont->SetHeight(sz);
	};

	// Show them
	if (psDeviceFlags.test(rsStatistic))
	{
		CSound_stats				snd_stat;
		::Sound->statistic			(&snd_stat,0);
		F.SetColor	(0xFFFFFFFF	);

		F.OutSet	(0,0);
		F.OutNext	("FPS/RFPS:    %3.1f/%3.1f",fFPS,fRFPS);
		F.OutNext	("TPS:         %2.2f M",	fTPS);
		m_pRender->OutData1(F);
#ifdef DEBUG
		F.OutSkip	();
		F.OutNext	("mapped:      %d",			g_file_mapped_memory);
		F.OutSkip	();
		m_pRender->OutData2(F);
#endif
		m_pRender->OutData3(F);
		F.OutSkip	();

#define PPP(a) (100.f*float(a)/float(EngineTOTAL.result))
		F.OutNext	("*** ENGINE:  %2.2fms",EngineTOTAL.result);	
		F.OutNext	("Memory:      %2.2fa",fMem_calls);
		F.OutNext	("uClients:    %2.2fms, %2.1f%%, crow(%d)/active(%d)/total(%d)",UpdateClient.result,PPP(UpdateClient.result),UpdateClient_crows,UpdateClient_active,UpdateClient_total);
		F.OutNext	("uSheduler:   %2.2fms, %2.1f%%",Sheduler.result,		PPP(Sheduler.result));
		F.OutNext	("uSheduler_L: %2.2fms",fShedulerLoad);
		F.OutNext	("uParticles:  Qstart[%d] Qactive[%d] Qdestroy[%d]",	Particles_starting,Particles_active,Particles_destroy);
		F.OutNext	("spInsert:    o[%.2fms, %2.1f%%], p[%.2fms, %2.1f%%]",	g_SpatialSpace->stat_insert.result, PPP(g_SpatialSpace->stat_insert.result),	g_SpatialSpacePhysic->stat_insert.result, PPP(g_SpatialSpacePhysic->stat_insert.result));
		F.OutNext	("spRemove:    o[%.2fms, %2.1f%%], p[%.2fms, %2.1f%%]",	g_SpatialSpace->stat_remove.result, PPP(g_SpatialSpace->stat_remove.result),	g_SpatialSpacePhysic->stat_remove.result, PPP(g_SpatialSpacePhysic->stat_remove.result));
		F.OutNext	("Physics:     %2.2fms, %2.1f%%",Physics.result,		PPP(Physics.result));	
		F.OutNext	("  collider:  %2.2fms", ph_collision.result);	
		F.OutNext	("  solver:    %2.2fms, %d",ph_core.result,ph_core.count);	
		F.OutNext	("aiThink:     %2.2fms, %d",AI_Think.result,AI_Think.count);	
		F.OutNext	("  aiRange:   %2.2fms, %d",AI_Range.result,AI_Range.count);
		F.OutNext	("  aiPath:    %2.2fms, %d",AI_Path.result,AI_Path.count);
		F.OutNext	("  aiNode:    %2.2fms, %d",AI_Node.result,AI_Node.count);
		F.OutNext	("aiVision:    %2.2fms, %d",AI_Vis.result,AI_Vis.count);
		F.OutNext	("  Query:     %2.2fms",	AI_Vis_Query.result);
		F.OutNext	("  RayCast:   %2.2fms",	AI_Vis_RayTests.result);
		F.OutSkip	();
								   
#undef  PPP
#define PPP(a) (100.f*float(a)/float(RenderTOTAL.result))
		F.OutNext	("*** RENDER:  %2.2fms",RenderTOTAL.result);
		F.OutNext	("R_CALC:      %2.2fms, %2.1f%%",RenderCALC.result,	PPP(RenderCALC.result));	
		F.OutNext	("  HOM:       %2.2fms, %d",RenderCALC_HOM.result,	RenderCALC_HOM.count);
		F.OutNext	("  Skeletons: %2.2fms, %d",Animation.result,		Animation.count);
		F.OutNext	("R_DUMP:      %2.2fms, %2.1f%%",RenderDUMP.result,	PPP(RenderDUMP.result));	
		F.OutNext	("  Wait-L:    %2.2fms",RenderDUMP_Wait.result);	
		F.OutNext	("  Wait-S:    %2.2fms",RenderDUMP_Wait_S.result);	
		F.OutNext	("  Skinning:  %2.2fms",RenderDUMP_SKIN.result);	
		F.OutNext	("  DT_Vis/Cnt:%2.2fms",RenderDUMP_DT_VIS.result,RenderDUMP_DT_Count);	
		F.OutNext	("  DT_Render: %2.2fms",RenderDUMP_DT_Render.result);	
		F.OutNext	("  DT_Cache:  %2.2fms",RenderDUMP_DT_Cache.result);	
		F.OutNext	("  Wallmarks: %2.2fms, %d/%d - %d",RenderDUMP_WM.result,RenderDUMP_WMS_Count,RenderDUMP_WMD_Count,RenderDUMP_WMT_Count);
		F.OutNext	("  Glows:     %2.2fms",RenderDUMP_Glows.result);	
		F.OutNext	("  Lights:    %2.2fms, %d",RenderDUMP_Lights.result,RenderDUMP_Lights.count);
		F.OutNext	("  RT:        %2.2fms, %d",RenderDUMP_RT.result,RenderDUMP_RT.count);
		F.OutNext	("  HUD:       %2.2fms",RenderDUMP_HUD.result);	
		F.OutNext   ("  Rain:      %2.2fms",RenderDUMP_Rain.result);
		F.OutNext	("  Projectors calc:    %2.2fms",RenderDUMP_Pcalc.result);
		F.OutNext	("  Shadow calc:    %2.2fms",RenderDUMP_Scalc.result);
		F.OutNext	("  Shadow render:  %2.2fms, %d",RenderDUMP_Srender.result,RenderDUMP_Srender.count);
		F.OutSkip	();
		F.OutNext	("*** SOUND:   %2.2fms",Sound.result);
		F.OutNext	("  TGT/SIM/E: %d/%d/%d",  snd_stat._rendered, snd_stat._simulated, snd_stat._events);
		F.OutNext	("  HIT/MISS:  %d/%d",  snd_stat._cache_hits, snd_stat._cache_misses);
		F.OutSkip	();
		F.OutNext	("Input:       %2.2fms",Input.result);
		
#ifdef DEBUG
		static float	r_ps = 0;
		static float	b_ps = 0;
		r_ps = .99f * r_ps + .01f * (CDB_RAY_TIMER(count) / CDB_RAY_TIMER(result));
		b_ps = .99f * b_ps + .01f * (CDB_BOX_TIMER(count) / CDB_BOX_TIMER(result));

		F.OutNext("CDB clRAY:       %2.2fms, %d, %2.0fK", CDB_RAY_TIMER(result), CDB_RAY_TIMER(count), r_ps);
		F.OutNext("CDB clBOX:       %2.2fms, %d, %2.0fK", CDB_BOX_TIMER(result), CDB_BOX_TIMER(count), b_ps);
		F.OutNext("CDB clFRUSTUM:   %2.2fms, %d", CDB_FRUSTUM_TIMER(result), CDB_FRUSTUM_TIMER(count));
#else
		F.OutSkip();
		F.OutNext("CDB Timers not implemented in not debugging mode!!!");
		F.OutSkip();
#endif

		F.OutSkip	();
		F.OutNext	("netClientRecv:   %2.2fms, %d",	netClient1.result,netClient1.count);
		F.OutNext	("netClientSend:   %2.2fms, %d",	netClient2.result,netClient2.count);
		F.OutNext	("netServer:   %2.2fms, %d",		netServer.result,netServer.count);
		F.OutNext	("netClientCompressor:   %2.2fms",	netClientCompressor.result);
		F.OutNext	("netServerCompressor:   %2.2fms",	netServerCompressor.result);
		F.OutSkip	();

		F.OutSkip	();
		F.OutNext	("Bullet Manager:      %2.2fms, %d",BulletManager.result,BulletManager.count);
		F.OutNext	("Actor Camera Update:      %2.2fms, %d", ActorCameraUpdate.result,ActorCameraUpdate.count);
		F.OutNext   ("Create List NetExport: %fms", CreateListNetExport.result);
		F.OutNext	("Send NetExport:      %2.2fms, %d",SendNetExport.result, SendNetExport.count);
#ifdef DEBUG_MEMORY_MANAGER
		F.OutSkip	();
		F.OutNext	("str: cmp[%3d], dock[%3d], qpc[%3d]",Memory.stat_strcmp,Memory.stat_strdock,CPU::qpc_counter);
		Memory.stat_strcmp	=	0		;
		Memory.stat_strdock	=	0		;
		CPU::qpc_counter	=	0		;
#else // DEBUG_MEMORY_MANAGER
		F.OutSkip	();
		F.OutNext	("qpc[%3d]",CPU::qpc_counter);
		CPU::qpc_counter	=	0		;
#endif // DEBUG_MEMORY_MANAGER

		F.OutSkip();
		m_pRender->OutData4(F);

		//////////////////////////////////////////////////////////////////////////
		// Renderer specific
		F.SetHeightI						(f_base_size);
		F.OutSet						(200,0);
		Render->Statistics				(&F);

		//////////////////////////////////////////////////////////////////////////
		// Game specific
		F.SetHeightI						(f_base_size);
		F.OutSet						(400,0);
		g_pGamePersistent->Statistics	(&F);

		//////////////////////////////////////////////////////////////////////////
		// process PURE STATS
		F.SetHeightI						(f_base_size);
		seqStats.Process				(rp_Stats);
		pFont->OnRender					();
	}

	if (DisplayFPSShow_)
	{
		CGameFont& fpsFont = *pFPSFont_;

		fpsFont.SetColor(0xFF8772A8);
		fpsFont.SetHeightI(0.02);
		fpsFont.SetAligment(CGameFont::alRight);
		float posx, posy;
		if (Device.dwWidth / Device.dwHeight < 1.5f) {			//posx = 1200.f * (Device.dwWidth / 1280.f);
			posx = 1180.f * (Device.dwWidth / 1280.f);
			posy = 10.f * (Device.dwHeight / 720.f);
		}
		else {
			//posx = 940.f * (Device.dwWidth / 1024.f);
			posx = 890.f * (Device.dwWidth / 1024.f);
			posy = 15.f * (Device.dwHeight / 768.f);
			fpsFont.SetHeightI(0.030);
		}
		fpsFont.OutSet(posx, posy);

		if (DisplayFPSShow_)
		{
			UpdateFPSCounterSkip += 1.f * Device.fTimeDelta;
			UpdateFPSMinute += 1.f * Device.fTimeDelta;
			if (UpdateFPSCounterSkip > 0.1f) {
				UpdateFPSCounterSkip = 0;
				iFPS = fFPS;
				fCounterTotalMinute += fFPS;
			}
			if (UpdateFPSMinute > 60.0f) {
				UpdateFPSMinute = 0;
				iTotalMinute = fCounterTotalMinute / 10;
				iAvrageMinute = fCounterTotalMinute / 600;
				fCounterTotalMinute = 0;

			}
			fpsFont.OutNext("      %i", iFPS);
			fpsFont.OutSkip();
			fpsFont.OutNext("A/MIN %i", iAvrageMinute);
			fpsFont.OutNext("T/MIN %i", iTotalMinute);
		}

		pFPSFont_->OnRender();
	}

	if( psDeviceFlags.test(rsStatistic) || psDeviceFlags.test(rsCameraPos) ){
		_draw_cam_pos					(pFont);
		pFont->OnRender					();
	}

#ifdef DEBUG
	//////////////////////////////////////////////////////////////////////////
	// PERF ALERT
	if (!g_bDisableRedText)
	{
		CGameFont&	F = *((CGameFont*)pFont);
		F.SetColor						(color_rgba(255,16,16,255));
		F.OutSet						(300,300);
		F.SetHeightI						(f_base_size*2);
		if (fFPS<30)					F.OutNext	("FPS       < 30:   %3.1f",	fFPS);
		m_pRender->GuardVerts(F);

		if (psDeviceFlags.test(rsStatistic))
		{
			m_pRender->GuardDrawCalls(F);
			if (RenderDUMP_DT_Count>1000)	F.OutNext	("DT_count  > 1000: %u",	RenderDUMP_DT_Count);
			F.OutSkip						();
			//if (fMem_calls>1500)			F.OutNext	("MMGR calls > 1500:%3.1f",	fMem_calls);
			if (Sheduler.result>3.f)		F.OutNext	("Update     > 3ms:	%3.1f",	Sheduler.result);
			if (UpdateClient.result>3.f)	F.OutNext	("UpdateCL   > 3ms: %3.1f",	UpdateClient.result);
			if (Physics.result>5.f)			F.OutNext	("Physics    > 5ms: %3.1f",	Physics.result);	
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Show errors
	if (!g_bDisableRedText && errors.size())
	{
		CGameFont&	F = *((CGameFont*)pFont);
		F.SetColor	(color_rgba(255,16,16,191));
		F.OutSet	(200,0);
		F.SetHeightI	(f_base_size);
#if 0
		for (u32 it=0; it<errors.size(); it++)
			F.OutNext("%s",errors[it].c_str());
#else
		for (u32 it=(u32)_max(int(0),(int)errors.size() - g_ErrorLineCount); it<errors.size(); it++)
			F.OutNext("%s",errors[it].c_str());
#endif
		F.OnRender	();
	}
#endif

	{
		EngineTOTAL.FrameStart		();	
		Sheduler.FrameStart			();	
		UpdateClient.FrameStart		();	
		Physics.FrameStart			();	
		ph_collision.FrameStart		();
		ph_core.FrameStart			();
		Animation.FrameStart		();	
		AI_Think.FrameStart			();
		AI_Range.FrameStart			();
		AI_Path.FrameStart			();
		AI_Node.FrameStart			();
		AI_Vis.FrameStart			();
		AI_Vis_Query.FrameStart		();
		AI_Vis_RayTests.FrameStart	();
		
		RenderTOTAL.FrameStart		();
		RenderCALC.FrameStart		();
		RenderCALC_HOM.FrameStart	();
		RenderDUMP.FrameStart		();	
		RenderDUMP_RT.FrameStart	();
		RenderDUMP_SKIN.FrameStart	();	
		RenderDUMP_Wait.FrameStart	();	
		RenderDUMP_Wait_S.FrameStart();	
		RenderDUMP_HUD.FrameStart	();	
		RenderDUMP_Rain.FrameStart  ();
		RenderDUMP_Glows.FrameStart	();	
		RenderDUMP_Lights.FrameStart();	
		RenderDUMP_WM.FrameStart	();	
		RenderDUMP_DT_VIS.FrameStart();	
		RenderDUMP_DT_Render.FrameStart();	
		RenderDUMP_DT_Cache.FrameStart();	
		RenderDUMP_Pcalc.FrameStart	();	
		RenderDUMP_Scalc.FrameStart	();	
		RenderDUMP_Srender.FrameStart();	
		
		Sound.FrameStart			();
		Input.FrameStart			();

		CDB_RAY_TIMER(FrameStart());
		CDB_BOX_TIMER(FrameStart());
		CDB_FRUSTUM_TIMER(FrameStart());
		
		netClient1.FrameStart		();
		netClient2.FrameStart		();
		netServer.FrameStart		();
		netClientCompressor.FrameStart();
		netServerCompressor.FrameStart();

		BulletManager.FrameStart();
		ActorCameraUpdate.FrameStart();
		CreateListNetExport.FrameStart();
		SendNetExport.FrameStart			();

		g_SpatialSpace->stat_insert.FrameStart		();
		g_SpatialSpace->stat_remove.FrameStart		();

		g_SpatialSpacePhysic->stat_insert.FrameStart();
		g_SpatialSpacePhysic->stat_remove.FrameStart();
	}
	dwSND_Played = dwSND_Allocated = 0;
	Particles_starting = Particles_active = Particles_destroy = 0;
}

void	_LogCallback				(LPCSTR string)
{
	if (string && '!'==string[0] && ' '==string[1])
		Device.Statistic->errors.push_back	(shared_str(string));
}

void CStats::OnDeviceCreate			()
{
	g_bDisableRedText				= strstr(Core.Params,"-xclsx");

//	if (!strstr(Core.Params, "-dedicated"))
#ifndef DEDICATED_SERVER
	pFont	= xr_new<CGameFont>("stat_font", CGameFont::fsDeviceIndependent);
	pFPSFont_ = xr_new<CGameFont>("hud_font_di", CGameFont::fsDeviceIndependent);
#endif
	
	// 
#ifdef DEBUG
	if (!g_bDisableRedText)			SetLogCB	(_LogCallback);
#endif
}

void CStats::OnDeviceDestroy		()
{
	xr_delete	(pFont);
	xr_delete(pFPSFont_);
}

void CStats::OnRender				()
{
#ifdef DEBUG
	if (g_stats_flags.is(st_sound)){
		CSound_stats_ext				snd_stat_ext;
		::Sound->statistic				(0,&snd_stat_ext);
		CSound_stats_ext::item_vec_it	_I = snd_stat_ext.items.begin();
		CSound_stats_ext::item_vec_it	_E = snd_stat_ext.items.end();
		for (;_I!=_E;_I++){
			const CSound_stats_ext::SItem& item = *_I;
			if (item._3D){
				m_pRender->SetDrawParams(&*Device.m_pRender);
				DU->DrawCross			(item.params.position, 0.5f, 0xFF0000FF, true );
				if (g_stats_flags.is(st_sound_min_dist))
					DU->DrawSphere		(Fidentity, item.params.position, item.params.min_distance, 0x400000FF,	0xFF0000FF, true, true);
				if (g_stats_flags.is(st_sound_max_dist))
					DU->DrawSphere		(Fidentity, item.params.position, item.params.max_distance, 0x4000FF00,	0xFF008000, true, true);
				xr_string out_txt		= (g_stats_flags.is(st_sound_info_name))?item.name.c_str():"";
				if (item.game_object){
					if (g_stats_flags.is(st_sound_ai_dist))
						DU->DrawSphere	(Fidentity, item.params.position, item.params.max_ai_distance, 0x80FF0000,0xFF800000,true,true);
					if (g_stats_flags.is(st_sound_info_object)){
						out_txt			+= "  (";
						out_txt			+= item.game_object->cNameSect().c_str();
						out_txt			+= ")";
					}
				}
				if (g_stats_flags.is_any(st_sound_info_name|st_sound_info_object))
					DU->OutText			(item.params.position, out_txt.c_str(),0xFFFFFFFF,0xFF000000);
			}
		}
	}
#endif
}
