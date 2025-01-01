#include "stdafx.h"
#include "igame_level.h"
#include "igame_persistent.h"

#include "x_ray.h"
#include "std_classes.h"
#include "customHUD.h"
#include "render.h"
#include "gamefont.h"
#include "xrLevel.h"
#include "CameraManager.h"
#include "xr_object.h"
#include "feel_sound.h"

ENGINE_API	IGame_Level*	g_pGameLevel	= NULL;
extern	BOOL g_bLoaded;

IGame_Level::IGame_Level	()
{
	m_pCameras					= xr_new<CCameraManager>(true);
	g_pGameLevel				= this;
	pLevel						= NULL;
	bReady						= false;
	pCurrentEntity				= NULL;
	pCurrentViewEntity			= NULL;
	pHUD						= NULL;
}

IGame_Level::~IGame_Level	()
{
	if(strstr(Core.Params,"-nes_texture_storing") )
		Device.m_pRender->ResourcesStoreNecessaryTextures();
//.	DEL_INSTANCE				( pHUD			);
	xr_delete					( pLevel		);

	// Render-level unload
	Render->level_Unload		();
	xr_delete					(m_pCameras);
	// Unregister
	Device.seqRender.Remove		(this);
	Device.seqFrame.Remove		(this);
	CCameraManager::ResetPP		();
}

void IGame_Level::net_Stop			()
{
	for (int i=0; i<6; i++)
		Objects.Update(true);
	// Destroy all objects
	Objects.Unload				( );
	IR_Release					( );

	bReady						= false;	
}

//-------------------------------------------------------------------------------------------
//extern CStatTimer				tscreate;
void __stdcall _sound_event		(ref_sound_data_ptr S, float range)
{
	if ( g_pGameLevel && S && S->feedback )	g_pGameLevel->SoundEvent_Register	(S,range);
}
static void __stdcall	build_callback	(Fvector* V, int Vcnt, CDB::TRI* T, int Tcnt, void* params)
{
	g_pGameLevel->Load_GameSpecific_CFORM( T, Tcnt );
}

BOOL IGame_Level::Load			(u32 dwNum) 
{
	// Initialize level data
	pApp->Level_Set				( dwNum );
	string_path					temp;
	if (!FS.exist(temp, "$level$", "level.ltx"))
		Debug.fatal	(DEBUG_INFO,"Can't find level configuration file '%s'.",temp);
	pLevel						= xr_new<CInifile>	( temp );
	
	// Open
//	g_pGamePersistent->LoadTitle	("st_opening_stream");
	g_pGamePersistent->LoadTitle();
	IReader* LL_Stream			= FS.r_open	("$level$","level");
	IReader	&fs					= *LL_Stream;

	// Header
	hdrLEVEL					H;
	fs.r_chunk_safe				(fsL_HEADER,&H,sizeof(H));
	R_ASSERT2					(XRCL_PRODUCTION_VERSION==H.XRLC_version,"Incompatible level version.");

	// CForms
//	g_pGamePersistent->LoadTitle	("st_loading_cform");
	g_pGamePersistent->LoadTitle();
	ObjectSpace.Load			( build_callback );
	//Sound->set_geometry_occ		( &Static );
	Sound->set_geometry_occ		(ObjectSpace.GetStaticModel	());
	Sound->set_handler			( _sound_event );

	pApp->LoadSwitch			();


	// HUD + Environment
//.	pHUD						= (CCustomHUD*)NEW_INSTANCE	(CLSID_HUDMANAGER);
	if(g_hud)
		pHUD					= g_hud;
	else
		pHUD					= (CCustomHUD*)NEW_INSTANCE	(CLSID_HUDMANAGER);

	// Render-level Load
	Render->level_Load			(LL_Stream);

	// Objects
	g_pGamePersistent->Environment().mods_load	();
	R_ASSERT					(Load_GameSpecific_Before());
	Objects.Load				();
	R_ASSERT					(Load_GameSpecific_After ());

	// Done
	FS.r_close					( LL_Stream );
	bReady						= true;
	if (!g_dedicated_server)	IR_Capture();
#ifndef DEDICATED_SERVER
	Device.seqRender.Add		(this);
#endif

	Device.seqFrame.Add			(this);

	return TRUE;
}

void IGame_Level::OnRender()
{
	Render->Calculate();
	Render->RenderFrame();
}

void	IGame_Level::OnFrame		( ) 
{
	// Log				("- level:on-frame: ",u32(Device.dwFrame));
//	if (_abs(Device.fTimeDelta)<EPS_S) return;

	// Update all objects
	VERIFY						(bReady);
	Objects.Update				( false );
	pHUD->OnFrame				( );

	// Ambience
	if (Sounds_Random.size() && (Device.dwTimeGlobal > Sounds_Random_dwNextTime))
	{
		Sounds_Random_dwNextTime		= Device.dwTimeGlobal + ::Random.randI	(10000,20000);
		Fvector	pos;
		pos.random_dir().normalize().mul(::Random.randF(30,100)).add	(Device.vCameraPosition);
		int		id						= ::Random.randI(Sounds_Random.size());
		if (Sounds_Random_Enabled)		{
			Sounds_Random[id].play_at_pos	(0,pos,0);
			Sounds_Random[id].set_volume	(1.f);
			Sounds_Random[id].set_range		(10,200);
		}
	}
}

// ==================================================================================================

void CServerInfo::AddItem( LPCSTR name_, LPCSTR value_, u32 color_ )
{
	shared_str s_name( name_ );
	AddItem( s_name, value_, color_ );
}

void CServerInfo::AddItem( shared_str& name_, LPCSTR value_, u32 color_ )
{
	SItem_ServerInfo it;
//	shared_str s_name = CStringTable().translate( name_ );
	
//	strcpy_s( it.name, s_name.c_str() );
	strcpy_s( it.name, name_.c_str() );
	strcat_s( it.name, " = " );
	strcat_s( it.name, value_ );
	it.color = color_;

	if ( data.size() < max_item )
	{
		data.push_back( it );
	}
}

void	IGame_Level::SoundEvent_Register(ref_sound_data_ptr S, float range)
{
	if (!g_bLoaded)									return;
	if (!S)											return;
	if (S->g_object && S->g_object->getDestroy()) { S->g_object = 0; return; }
	if (0 == S->feedback)								return;

	clamp(range, 0.1f, 500.f);

	const CSound_params* p = S->feedback->get_params();
	Fvector snd_position = p->position;
	if (S->feedback->is_2D()) {
		snd_position.add(Sound->listener_position());
	}

	VERIFY(p && _valid(range));
	range = _min(range, p->max_ai_distance);
	VERIFY(_valid(snd_position));
	VERIFY(_valid(p->max_ai_distance));
	VERIFY(_valid(p->volume));

	// Query objects
	Fvector					bb_size = { range,range,range };
	g_SpatialSpace->q_box(snd_ER, 0, STYPE_REACTTOSOUND, snd_position, bb_size);

	// Iterate
	xr_vector<ISpatial*>::iterator	it = snd_ER.begin();
	xr_vector<ISpatial*>::iterator	end = snd_ER.end();
	for (; it != end; it++) {
		Feel::Sound* L = (*it)->dcast_FeelSound();
		if (0 == L)			continue;
		CObject* CO = (*it)->dcast_CObject();	VERIFY(CO);
		if (CO->getDestroy()) continue;

		// Energy and signal
		VERIFY(_valid((*it)->spatial.sphere.P));
		float dist = snd_position.distance_to((*it)->spatial.sphere.P);
		if (dist > p->max_ai_distance) continue;
		VERIFY(_valid(dist));
		VERIFY2(!fis_zero(p->max_ai_distance), S->handle->file_name());
		float Power = (1.f - dist / p->max_ai_distance) * p->volume;
		VERIFY(_valid(Power));
		if (Power > EPS_S) {
			float occ = Sound->get_occlusion_to((*it)->spatial.sphere.P, snd_position);
			VERIFY(_valid(occ));
			Power *= occ;
			if (Power > EPS_S) {
				_esound_delegate	D = { L, S, Power };
				snd_Events.push_back(D);
			}
		}
	}
	snd_ER.clear_not_free();
}

void	IGame_Level::SoundEvent_Dispatch()
{
	while (!snd_Events.empty()) {
		_esound_delegate& D = snd_Events.back();
		VERIFY(D.dest && D.source);
		if (D.source->feedback) {
			D.dest->feel_sound_new(
				D.source->g_object,
				D.source->g_type,
				D.source->g_userdata,

				D.source->feedback->is_2D() ? Device.vCameraPosition :
				D.source->feedback->get_params()->position,
				D.power
			);
		}
		snd_Events.pop_back();
	}
}
