////////////////////////////////////////////////////////////////////////////
//	Module 		: level_script.cpp
//	Created 	: 28.06.2004
//  Modified 	: 28.06.2004
//	Author		: Dmitriy Iassenev
//	Description : Level script export
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "level.h"
#include "actor.h"
#include "script_game_object.h"
#include "patrol_path_storage.h"
#include "xrServer.h"
#include "client_spawn_manager.h"
#include "../xr_3da/igame_persistent.h"
#include "game_cl_base.h"
#include "ui/UIDialogWnd.h"
#include "date_time.h"
#include "ai_space.h"
#include "gamepersistent.h"
#include "level_graph.h"
#include "PHCommander.h"
#include "PHScriptCall.h"
#include "UIGameCustom.h"
#include "game_cl_single.h"
#include "map_manager.h"
#include "map_location.h"
#include "physics_world_scripted.h"

using namespace luabind;

LPCSTR command_line	()
{
	return		(Core.Params);
}

#ifdef DEBUG
void check_object(CScriptGameObject *object)
{
	try {
		Msg	("check_object %s",object->Name());
	}
	catch(...) {
		object = object;
	}
}

CScriptGameObject *tpfGetActor()
{
	static bool first_time = true;
	if (first_time)
		ai().script_engine().script_log(eLuaMessageTypeError,"Do not use level.actor function!");
	first_time = false;
	
	CActor *l_tpActor = smart_cast<CActor*>(Level().CurrentEntity());
	if (l_tpActor)
		return	(smart_cast<CGameObject*>(l_tpActor)->lua_game_object());
	else
		return	(0);
}

CScriptGameObject *get_object_by_name(LPCSTR caObjectName)
{
	static bool first_time = true;
	if (first_time)
		ai().script_engine().script_log(eLuaMessageTypeError,"Do not use level.object function!");
	first_time = false;
	
	CGameObject		*l_tpGameObject	= smart_cast<CGameObject*>(Level().Objects.FindObjectByName(caObjectName));
	if (l_tpGameObject)
		return		(l_tpGameObject->lua_game_object());
	else
		return		(0);
}
#endif

CScriptGameObject *get_object_by_id(u32 id)
{
	CGameObject* pGameObject = smart_cast<CGameObject*>(Level().Objects.net_Find(id));
	if(!pGameObject)
		return NULL;

	return pGameObject->lua_game_object();
}

LPCSTR get_weather	()
{
	return			(*g_pGamePersistent->Environment().GetWeather());
}

LPCSTR get_weather_prev()
{
	return			(*g_pGamePersistent->Environment().GetPrevWeather());
}

void set_weather	(LPCSTR weather_name, bool forced)
{
	g_pGamePersistent->Environment().SetWeather(weather_name, forced);
}

void set_weather_next(LPCSTR weather_name) {
	g_pGamePersistent->Environment().SetWeatherNext(weather_name);
}

bool set_weather_fx(LPCSTR weather_name)
{
	return		(g_pGamePersistent->Environment().SetWeatherFX(weather_name));
}
bool is_wfx_playing	()
{
	return			(g_pGamePersistent->Environment().IsWFXPlaying());
}

void set_time_factor(float time_factor)
{
	if (!OnServer())
		return;

	Level().Server->game->SetGameTimeFactor(time_factor);
	GamePersistent().Environment().SetGameTime(Level().GetEnvironmentGameDayTimeSec(), Level().game->GetEnvironmentGameTimeFactor());
}

float get_time_factor()
{
	return			(Level().GetGameTimeFactor());
}

void set_game_difficulty(ESingleGameDifficulty dif)
{
	g_SingleGameDifficulty		= dif;
	game_cl_Single* game		= smart_cast<game_cl_Single*>(Level().game); VERIFY(game);
	game->OnDifficultyChanged	();
}
ESingleGameDifficulty get_game_difficulty()
{
	return g_SingleGameDifficulty;
}

u32 get_time_days()
{
	u32 year = 0, month = 0, day = 0, hours = 0, mins = 0, secs = 0, milisecs = 0;
	split_time(Level().GetGameTime(), year, month, day, hours, mins, secs, milisecs);
	return			day;
}

u32 get_time_hours()
{
	u32 year = 0, month = 0, day = 0, hours = 0, mins = 0, secs = 0, milisecs = 0;
	split_time(Level().GetGameTime(), year, month, day, hours, mins, secs, milisecs);
	return			hours;
}

u32 get_time_minutes()
{
	u32 year = 0, month = 0, day = 0, hours = 0, mins = 0, secs = 0, milisecs = 0;
	split_time(Level().GetGameTime(), year, month, day, hours, mins, secs, milisecs);
	return			mins;
}

float cover_in_direction(u32 level_vertex_id, const Fvector &direction)
{
	float			y,p;
	direction.getHP	(y,p);
	return			(ai().level_graph().cover_in_direction(y,level_vertex_id));
}

float rain_factor()
{
	return			(g_pGamePersistent->Environment().CurrentEnv->rain_density);
}

u32	vertex_in_direction(u32 level_vertex_id, Fvector direction, float max_distance)
{
	direction.normalize_safe();
	direction.mul	(max_distance);
	Fvector			start_position = ai().level_graph().vertex_position(level_vertex_id);
	Fvector			finish_position = Fvector(start_position).add(direction);
	u32				result = u32(-1);
	ai().level_graph().farthest_vertex_in_direction(level_vertex_id,start_position,finish_position,result,0);
	return			(ai().level_graph().valid_vertex_id(result) ? result : level_vertex_id);
}

Fvector vertex_position(u32 level_vertex_id)
{
	return			(ai().level_graph().vertex_position(level_vertex_id));
}

void map_add_object_spot(u16 id, LPCSTR spot_type, LPCSTR text)
{
	CMapLocation* ml = Level().MapManager().AddMapLocation(spot_type,id);
	if( xr_strlen(text) )
			ml->SetHint(text);
}

void map_add_object_spot_ser(u16 id, LPCSTR spot_type, LPCSTR text)
{
	CMapLocation* ml = Level().MapManager().AddMapLocation(spot_type,id);
	if( xr_strlen(text) )
			ml->SetHint(text);

	ml->SetSerializable(true);
}

void map_change_spot_hint(u16 id, LPCSTR spot_type, LPCSTR text)
{
	CMapLocation* ml	= Level().MapManager().GetMapLocation(spot_type, id);
	if(!ml)				return;
	ml->SetHint			(text);
}

void map_remove_object_spot(u16 id, LPCSTR spot_type)
{
	Level().MapManager().RemoveMapLocation(spot_type, id);
}

u16 map_has_object_spot(u16 id, LPCSTR spot_type)
{
	return Level().MapManager().HasMapLocation(spot_type, id);
}

bool patrol_path_exists(LPCSTR patrol_path)
{
	return		(!!ai().patrol_paths().path(patrol_path,true));
}

LPCSTR get_name()
{
	return		(*Level().name());
}

void prefetch_sound	(LPCSTR name)
{
	Level().PrefetchSound(name);
}


CClientSpawnManager	&get_client_spawn_manager()
{
	return		(Level().client_spawn_manager());
}

void start_stop_menu(CUIDialogWnd* pDialog, bool bDoHideIndicators)
{
	if (!pDialog->IsShown())
		pDialog->ShowDialog(bDoHideIndicators);
	else
		pDialog->HideDialog();
}


void add_dialog_to_render(CUIDialogWnd* pDialog)
{
	CurrentGameUI()->AddDialogToRender(pDialog);
}

void remove_dialog_to_render(CUIDialogWnd* pDialog)
{
	CurrentGameUI()->RemoveDialogToRender(pDialog);
}

CUIDialogWnd* main_input_receiver()
{
	return CurrentGameUI()->TopInputReceiver();
}
#include "UIGameCustom.h"
void hide_indicators()
{
	if(CurrentGameUI())
	{
		CurrentGameUI()->HideShownDialogs();
		CurrentGameUI()->ShowGameIndicators(false);
		CurrentGameUI()->ShowCrosshair(false);
	}
}

void show_indicators()
{
	if (CurrentGameUI())
	{
		CurrentGameUI()->ShowGameIndicators(true);
		CurrentGameUI()->ShowCrosshair(true);
	}
}


bool is_level_present()
{
	return (!!g_pGameLevel);
}

void add_call(const luabind::functor<bool> &condition,const luabind::functor<void> &action)
{
	luabind::functor<bool>		_condition = condition;
	luabind::functor<void>		_action = action;
	CPHScriptCondition	* c=xr_new<CPHScriptCondition>(_condition);
	CPHScriptAction		* a=xr_new<CPHScriptAction>(_action);
	Level().ph_commander_scripts().add_call(c,a);
}

void remove_call(const luabind::functor<bool> &condition,const luabind::functor<void> &action)
{
	CPHScriptCondition	c(condition);
	CPHScriptAction		a(action);
	Level().ph_commander_scripts().remove_call(&c,&a);
}

void add_call(const luabind::object &lua_object, LPCSTR condition,LPCSTR action)
{
//	try{	
//		CPHScriptObjectCondition	*c=xr_new<CPHScriptObjectCondition>(lua_object,condition);
//		CPHScriptObjectAction		*a=xr_new<CPHScriptObjectAction>(lua_object,action);
		luabind::functor<bool>		_condition = object_cast<luabind::functor<bool> >(lua_object[condition]);
		luabind::functor<void>		_action = object_cast<luabind::functor<void> >(lua_object[action]);
		CPHScriptObjectConditionN	*c=xr_new<CPHScriptObjectConditionN>(lua_object,_condition);
		CPHScriptObjectActionN		*a=xr_new<CPHScriptObjectActionN>(lua_object,_action);
		Level().ph_commander_scripts().add_call_unique(c,c,a,a);
//	}
//	catch(...)
//	{
//		Msg("add_call excepted!!");
//	}
}

void remove_call(const luabind::object &lua_object, LPCSTR condition,LPCSTR action)
{
	CPHScriptObjectCondition	c(lua_object,condition);
	CPHScriptObjectAction		a(lua_object,action);
	Level().ph_commander_scripts().remove_call(&c,&a);
}

void add_call(const luabind::object &lua_object, const luabind::functor<bool> &condition,const luabind::functor<void> &action)
{

	CPHScriptObjectConditionN	*c=xr_new<CPHScriptObjectConditionN>(lua_object,condition);
	CPHScriptObjectActionN		*a=xr_new<CPHScriptObjectActionN>(lua_object,action);
	Level().ph_commander_scripts().add_call(c,a);
}

void remove_call(const luabind::object &lua_object, const luabind::functor<bool> &condition,const luabind::functor<void> &action)
{
	CPHScriptObjectConditionN	c(lua_object,condition);
	CPHScriptObjectActionN		a(lua_object,action);
	Level().ph_commander_scripts().remove_call(&c,&a);
}

void remove_calls_for_object(const luabind::object &lua_object)
{
	CPHSriptReqObjComparer c(lua_object);
	Level().ph_commander_scripts().remove_calls(&c);
}

cphysics_world_scripted* physics_world_scripted()
{
	return	get_script_wrapper<cphysics_world_scripted>(*physics_world());
}
CEnvironment *environment()
{
	return		(g_pGamePersistent->pEnvironment);
}

CEnvDescriptor *current_environment(CEnvironment *self)
{
	return		(self->CurrentEnv);
}
extern bool g_bDisableAllInput;
void disable_input()
{
	g_bDisableAllInput = true;
}
void enable_input()
{
	g_bDisableAllInput = false;
}

void spawn_phantom(const Fvector &position)
{
	Level().spawn_item("m_phantom", position, u32(-1), u16(-1), false);
}

Fbox get_bounding_volume()
{
	return Level().ObjectSpace.GetBoundingVolume();
}

void iterate_sounds					(LPCSTR prefix, u32 max_count, const CScriptCallbackEx<void> &callback)
{
	for (int j=0, N = _GetItemCount(prefix); j<N; ++j) {
		string_path					fn, s;
		LPSTR						S = (LPSTR)&s;
		_GetItem					(prefix,j,S);
		if (FS.exist(fn,"$game_sounds$",S,".ogg"))
			callback				(prefix);

		for (u32 i=0; i<max_count; ++i)
		{
			string_path					name;
			sprintf_s					(name,"%s%d",S,i);
			if (FS.exist(fn,"$game_sounds$",name,".ogg"))
				callback			(name);
		}
	}
}

void iterate_sounds1				(LPCSTR prefix, u32 max_count, luabind::functor<void> functor)
{
	CScriptCallbackEx<void>		temp;
	temp.set					(functor);
	iterate_sounds				(prefix,max_count,temp);
}

void iterate_sounds2				(LPCSTR prefix, u32 max_count, luabind::object object, luabind::functor<void> functor)
{
	CScriptCallbackEx<void>		temp;
	temp.set					(functor,object);
	iterate_sounds				(prefix,max_count,temp);
}

#include "actoreffector.h"
float add_cam_effector(LPCSTR fn, int id, bool cyclic, LPCSTR cb_func)
{
	CAnimatorCamEffectorScriptCB* e		= xr_new<CAnimatorCamEffectorScriptCB>(cb_func);
	e->SetType					((ECamEffectorType)id);
	e->SetCyclic				(cyclic);
	e->Start					(fn);
	Actor()->Cameras().AddCamEffector(e);
	return						e->GetAnimatorLength();
}

float add_cam_effector2(LPCSTR fn, int id, bool cyclic, LPCSTR cb_func)
{
	CAnimatorCamEffectorScriptCB* e		= xr_new<CAnimatorCamEffectorScriptCB>(cb_func);
	e->m_bAbsolutePositioning	= true;
	e->SetType					((ECamEffectorType)id);
	e->SetCyclic				(cyclic);
	e->Start					(fn);
	Actor()->Cameras().AddCamEffector(e);
	return						e->GetAnimatorLength();
}

void remove_cam_effector(int id)
{
	Actor()->Cameras().RemoveCamEffector((ECamEffectorType)id );
}
		
float get_snd_volume()
{
	return psSoundVFactor;
}

void set_snd_volume(float v)
{
	psSoundVFactor = v;
	clamp(psSoundVFactor,0.0f,1.0f);
}
#include "actor_statistic_mgr.h"
void add_actor_points(LPCSTR sect, LPCSTR detail_key, int cnt, int pts)
{
	return Actor()->StatisticMgr().AddPoints(sect, detail_key, cnt, pts);
}

void add_actor_points_str(LPCSTR sect, LPCSTR detail_key, LPCSTR str_value)
{
	return Actor()->StatisticMgr().AddPoints(sect, detail_key, str_value);
}

int get_actor_points(LPCSTR sect)
{
	return Actor()->StatisticMgr().GetSectionPoints(sect);
}
extern int get_actor_ranking();
extern void add_human_to_top_list		(u16 id);
extern void remove_human_from_top_list	(u16 id);


void add_complex_effector(LPCSTR section, int id)
{
	AddEffector(Actor(),id, section);
}

void remove_complex_effector(int id)
{
	RemoveEffector(Actor(),id);
}

#include "postprocessanimator.h"
void add_pp_effector(LPCSTR fn, int id, bool cyclic)
{
	CPostprocessAnimator* pp		= xr_new<CPostprocessAnimator>(id, cyclic);
	pp->Load						(fn);
	Actor()->Cameras().AddPPEffector	(pp);
}

void remove_pp_effector(int id)
{
	CPostprocessAnimator*	pp	= smart_cast<CPostprocessAnimator*>(Actor()->Cameras().GetPPEffector((EEffectorPPType)id));

	if(pp) pp->Stop(1.0f);

}

void set_pp_effector_factor(int id, float f, float f_sp)
{
	CPostprocessAnimator*	pp	= smart_cast<CPostprocessAnimator*>(Actor()->Cameras().GetPPEffector((EEffectorPPType)id));

	if(pp) pp->SetDesiredFactor(f,f_sp);
}

void set_pp_effector_factor2(int id, float f)
{
	CPostprocessAnimator*	pp	= smart_cast<CPostprocessAnimator*>(Actor()->Cameras().GetPPEffector((EEffectorPPType)id));

	if(pp) pp->SetCurrentFactor(f);
}

#include "relation_registry.h"

 int g_community_goodwill(LPCSTR _community, int _entity_id)
 {
	 CHARACTER_COMMUNITY c;
	 c.set					(_community);

 	return RELATION_REGISTRY().GetCommunityGoodwill(c.index(), u16(_entity_id));
 }

void g_set_community_goodwill(LPCSTR _community, int _entity_id, int val)
{
	 CHARACTER_COMMUNITY	c;
	 c.set					(_community);
	RELATION_REGISTRY().SetCommunityGoodwill(c.index(), u16(_entity_id), val);
}

void g_change_community_goodwill(LPCSTR _community, int _entity_id, int val)
{
	 CHARACTER_COMMUNITY	c;
	 c.set					(_community);
	RELATION_REGISTRY().ChangeCommunityGoodwill(c.index(), u16(_entity_id), val);
}

#pragma optimize("s",on)
void CLevel::script_register(lua_State *L)
{
	class_<CEnvDescriptor>("CEnvDescriptor")
		.def_readonly("fog_density",			&CEnvDescriptor::fog_density)
		.def_readonly("far_plane",				&CEnvDescriptor::far_plane),

	class_<CEnvironment>("CEnvironment")
		.def("current",							current_environment);

	module(L,"level")
	[
		// obsolete\deprecated
		def("object_by_id",						get_object_by_id),
#ifdef DEBUG
		def("debug_object",						get_object_by_name),
		def("debug_actor",						tpfGetActor),
		def("check_object",						check_object),
#endif
		
		def("get_weather",						get_weather),
		def("get_weather_prev", get_weather_prev),
		def("set_weather",						set_weather),
		def("set_weather_next", set_weather_next),
		def("set_weather_fx",					set_weather_fx),
		def("is_wfx_playing",					is_wfx_playing),

		def("environment",						environment),
		
		def("set_time_factor",					set_time_factor),
		def("get_time_factor",					get_time_factor),

		def("set_game_difficulty",				set_game_difficulty),
		def("get_game_difficulty",				get_game_difficulty),
		
		def("get_time_days",					get_time_days),
		def("get_time_hours",					get_time_hours),
		def("get_time_minutes",					get_time_minutes),

		def("cover_in_direction",				cover_in_direction),
		def("vertex_in_direction",				vertex_in_direction),
		def("rain_factor",						rain_factor),
		def("patrol_path_exists",				patrol_path_exists),
		def("vertex_position",					vertex_position),
		def("name",								get_name),
		def("prefetch_sound",					prefetch_sound),

		def("client_spawn_manager",				get_client_spawn_manager),

		def("map_add_object_spot_ser",			map_add_object_spot_ser),
		def("map_add_object_spot",				map_add_object_spot),
		def("map_remove_object_spot",			map_remove_object_spot),
		def("map_has_object_spot",				map_has_object_spot),
		def("map_change_spot_hint",				map_change_spot_hint),

		def("start_stop_menu",					start_stop_menu),
		def("add_dialog_to_render",				add_dialog_to_render),
		def("remove_dialog_to_render",			remove_dialog_to_render),
		def("main_input_receiver",				main_input_receiver),
		def("hide_indicators",					hide_indicators),
		def("show_indicators",					show_indicators),
		def("add_call",							((void (*) (const luabind::functor<bool> &,const luabind::functor<void> &)) &add_call)),
		def("add_call",							((void (*) (const luabind::object &,const luabind::functor<bool> &,const luabind::functor<void> &)) &add_call)),
		def("add_call",							((void (*) (const luabind::object &, LPCSTR, LPCSTR)) &add_call)),
		def("remove_call",						((void (*) (const luabind::functor<bool> &,const luabind::functor<void> &)) &remove_call)),
		def("remove_call",						((void (*) (const luabind::object &,const luabind::functor<bool> &,const luabind::functor<void> &)) &remove_call)),
		def("remove_call",						((void (*) (const luabind::object &, LPCSTR, LPCSTR)) &remove_call)),
		def("remove_calls_for_object",			remove_calls_for_object),
		def("present",							is_level_present),
		def("disable_input",					disable_input),
		def("enable_input",						enable_input),
		def("spawn_phantom",					spawn_phantom),

		def("get_bounding_volume",				get_bounding_volume),

		def("iterate_sounds",					&iterate_sounds1),
		def("iterate_sounds",					&iterate_sounds2),
		def("physics_world",					&physics_world),
		def("get_snd_volume",					&get_snd_volume),
		def("set_snd_volume",					&set_snd_volume),
		def("add_cam_effector",					&add_cam_effector),
		def("add_cam_effector2",				&add_cam_effector2),
		def("remove_cam_effector",				&remove_cam_effector),
		def("add_pp_effector",					&add_pp_effector),
		def("set_pp_effector_factor",			&set_pp_effector_factor),
		def("set_pp_effector_factor",			&set_pp_effector_factor2),
		def("remove_pp_effector",				&remove_pp_effector),

		def("add_complex_effector",				&add_complex_effector),
		def("remove_complex_effector",			&remove_complex_effector),
		
		def("game_id",							&GameID)
	],
	
	module(L,"actor_stats")
	[
		def("add_points",						&add_actor_points),
		def("add_points_str",					&add_actor_points_str),
		def("get_points",						&get_actor_points),
		def("add_to_ranking",					&add_human_to_top_list),
		def("remove_from_ranking",				&remove_human_from_top_list),
		def("get_actor_ranking",				&get_actor_ranking)
	];

	module(L)
	[
		def("command_line",						&command_line),
		def("IsGameTypeSingle",					&IsGameTypeSingle)
	];

	module(L,"relation_registry")
	[
		def("community_goodwill",				&g_community_goodwill),
		def("set_community_goodwill",			&g_set_community_goodwill),
		def("change_community_goodwill",		&g_change_community_goodwill)
	];
}
