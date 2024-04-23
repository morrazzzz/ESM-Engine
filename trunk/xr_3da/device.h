#pragma once

// Note:
// ZNear - always 0.0f
// ZFar  - always 1.0f

#include "pure.h"
#include "ftimer.h"
#include "stats.h"

#define VIEWPORT_NEAR  0.2f

#define DEVICE_RESET_PRECACHE_FRAME_COUNT 10

#include "../Include/xrRender/RenderDeviceRender.h"

#pragma pack(push,4)

class IRenderDevice
{
public:
	virtual		CStatsPhysics* _BCL		StatPhysics() = 0;
	virtual				void	_BCL		AddSeqFrame(pureFrame* f, bool mt) = 0;
	virtual				void	_BCL		RemoveSeqFrame(pureFrame* f) = 0;
};

class ENGINE_API CRenderDeviceData
{

public:
	u32										dwWidth;
	u32										dwHeight;

	u32										dwPrecacheFrame;
	BOOL									b_is_Ready;
	BOOL									b_is_Active;
public:

	// Engine flow-control
	u32										dwFrame;

	float									fTimeDelta;
	float									fTimeGlobal;
	u32										dwTimeDelta;
	u32										dwTimeGlobal;
	u32										dwTimeContinual;

	Fvector									vCameraPosition;
	Fvector									vCameraDirection;
	Fvector									vCameraTop;
	Fvector									vCameraRight;

	Fmatrix									mView;
	Fmatrix									mProject;
	Fmatrix									mFullTransform;

	// Copies of corresponding members. Used for synchronization.
	Fvector									vCameraPosition_saved;

	Fmatrix									mView_saved;
	Fmatrix									mProject_saved;
	Fmatrix									mFullTransform_saved;

	float									fFOV;
	float									fASPECT;
protected:

	u32										Timer_MM_Delta;
	CTimer_paused							Timer;
	CTimer_paused							TimerGlobal;
public:

	// Registrators
	CRegistrator	<pureRender			>			seqRender;
	CRegistrator	<pureAppActivate	>			seqAppActivate;
	CRegistrator	<pureAppDeactivate	>			seqAppDeactivate;
	CRegistrator	<pureAppStart		>			seqAppStart;
	CRegistrator	<pureAppEnd			>			seqAppEnd;
	CRegistrator	<pureFrame			>			seqFrame;
	CRegistrator	<pureScreenResolutionChanged>	seqResolutionChanged;

	HWND									m_hWnd;
	//	CStats*									Statistic;

};

class	ENGINE_API CRenderDeviceBase :
	public IRenderDevice,
	public CRenderDeviceData
{
public:
};

#pragma pack(pop)
// refs
class ENGINE_API CRenderDevice : public CRenderDeviceBase
{
private:
    // Main objects used for creating and rendering the 3D scene
    u32										m_dwWindowStyle;
    RECT									m_rcWindowBounds;
    RECT									m_rcWindowClient;

	//u32										Timer_MM_Delta;
	//CTimer_paused							Timer;
	//CTimer_paused							TimerGlobal;
	CTimer									TimerMM;

	void									_Create		(LPCSTR shName);
	void									_Destroy	(BOOL	bKeepTextures);
	void									_SetupStates();
public:
    //HWND									m_hWnd;
	LRESULT									MsgProc		(HWND,UINT,WPARAM,LPARAM);

	//u32										dwFrame;
	//u32										dwPrecacheFrame;
	u32										dwPrecacheTotal;

	//u32										dwWidth, dwHeight;
	float									fWidth_2, fHeight_2;
	//BOOL									b_is_Ready;
	//BOOL									b_is_Active;
	void									OnWM_Activate(WPARAM wParam, LPARAM lParam);
public:
	
    BOOL									m_bNearer; 
    
	IRenderDeviceRender* m_pRender;

	void									SetNearer	(BOOL enabled)
	{
		if (enabled&&!m_bNearer){
			m_bNearer						= TRUE;
			mProject._43					-= EPS_L;
		}else if (!enabled&&m_bNearer){
			m_bNearer						= FALSE;
			mProject._43					+= EPS_L;
		}
		m_pRender->SetCacheXform(mView, mProject);
	}
public:
	// Registrators
	//CRegistrator	<pureRender			>			seqRender;
	//CRegistrator	<pureAppActivate	>			seqAppActivate;
	//CRegistrator	<pureAppDeactivate	>			seqAppDeactivate;
	//CRegistrator	<pureAppStart		>			seqAppStart;
	//CRegistrator	<pureAppEnd			>			seqAppEnd;
	//CRegistrator	<pureFrame			>			seqFrame;
	CRegistrator	<pureFrame			>			seqFrameMT;
	CRegistrator	<pureDeviceReset	>			seqDeviceReset;
	xr_vector		<fastdelegate::FastDelegate0<> >	seqParallel;

	// Dependent classes
 
	CStats*									Statistic;
	

	// Engine flow-control
	//float									fTimeDelta;
	//float									fTimeGlobal;
	//u32										dwTimeDelta;
	//u32										dwTimeGlobal;
	//u32										dwTimeContinual;

	// Cameras & projection
	//Fvector									vCameraPosition;
	//Fvector									vCameraDirection;
	//Fvector									vCameraTop;
	//Fvector									vCameraRight;

	//Fmatrix									mView;
	//Fmatrix									mProject;
	//Fmatrix									mFullTransform;

	Fmatrix mInvFullTransform;

	//float									fFOV;
	//float									fASPECT;
	
	CRenderDevice			()
		#ifdef PROFILE_CRITICAL_SECTIONS
			: mt_csEnter(MUTEX_PROFILE_ID(CRenderDevice::mt_csEnter))
			,mt_csLeave(MUTEX_PROFILE_ID(CRenderDevice::mt_csLeave))
		#endif // PROFILE_CRITICAL_SECTIONS
	{
	    m_hWnd              = NULL;
		b_is_Active			= FALSE;
		b_is_Ready			= FALSE;
		Timer.Start			();
		m_bNearer			= FALSE;
		m_pRender = nullptr;
	};

	void	Pause							(BOOL bOn, BOOL bTimer, BOOL bSound, LPCSTR reason);
	BOOL	Paused							();

	// Scene control
	void PreCache							(u32 amount, bool b_draw_loadscreen, bool b_wait_user_input);
	BOOL Begin								();
	void Clear								();
	void End								();
	void FrameMove							();
	
	void overdrawBegin						();
	void overdrawEnd						();

	// Mode control
	void DumpFlags							();
	IC	 CTimer_paused* GetTimerGlobal		()	{ return &TimerGlobal;								}
	u32	 TimerAsync							()	{ return TimerGlobal.GetElapsed_ms();				}
	u32	 TimerAsync_MMT						()	{ return TimerMM.GetElapsed_ms() +	Timer_MM_Delta; }

	// Creation & Destroying
	void ConnectToRender();
	void Create								(void);
	void Run								(void);
	void Destroy							(void);
	void Reset								(bool precache = true);

	void Initialize							(void);
	void ShutDown							(void);

public:
	void time_factor						(const float &time_factor)
	{
		Timer.time_factor		(time_factor);
		TimerGlobal.time_factor	(time_factor);
	}
	
	IC	const float &time_factor			() const
	{
		VERIFY					(Timer.time_factor() == TimerGlobal.time_factor());
		return					(Timer.time_factor());
	}

	// Multi-threading
	xrCriticalSection	mt_csEnter;
	xrCriticalSection	mt_csLeave;
	volatile BOOL		mt_bMustExit;

	ICF		void			remove_from_seq_parallel	(const fastdelegate::FastDelegate0<> &delegate)
	{
		xr_vector<fastdelegate::FastDelegate0<> >::iterator I = std::find(
			seqParallel.begin(),
			seqParallel.end(),
			delegate
		);
		if (I != seqParallel.end())
			seqParallel.erase	(I);
	}

public:
	void xr_stdcall		on_idle();
	bool xr_stdcall		on_message(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& result);

private:
	void					message_loop();
	virtual		void			_BCL	AddSeqFrame(pureFrame* f, bool mt);
	virtual		void			_BCL	RemoveSeqFrame(pureFrame* f);
	virtual		CStatsPhysics* _BCL	StatPhysics() { return  Statistic; }
};

extern		ENGINE_API		CRenderDevice		Device;

#ifndef	_EDITOR
#define	RDEVICE	Device
#else
#define RDEVICE	EDevice
#endif


extern		ENGINE_API		bool				g_bBenchmark;

typedef fastdelegate::FastDelegate0<bool>		LOADING_EVENT;
extern	ENGINE_API xr_list<LOADING_EVENT>		g_loading_events;

class ENGINE_API CLoadScreenRenderer :public pureRender
{
public:
					CLoadScreenRenderer	();
	void			start				(bool b_user_input);
	void			stop				();
	virtual void	OnRender			();

	bool			b_registered;
	bool			b_need_user_input;
};
extern ENGINE_API CLoadScreenRenderer load_screen_renderer;