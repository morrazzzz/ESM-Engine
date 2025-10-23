#include "stdafx.h"
#include "game_sv_single.h"
#include "xrserver_objects_alife_monsters.h"
#include "alife_simulator.h"
#include "alife_object_registry.h"
#include "alife_graph_registry.h"
#include "alife_time_manager.h"
#include "net_utils.h"
#include "object_broker.h"
#include "gamepersistent.h"
#include "xrServer.h"
#include "..\xr_3da\x_ray.h"

game_sv_Single::game_sv_Single			()
{
	m_alife_simulator			= NULL;
};

game_sv_Single::~game_sv_Single			()
{
	delete_data					(m_alife_simulator);
}

void	game_sv_Single::Create			(shared_str& options)
{
	inherited::Create					(options);

	if (strstr(*options,"/alife"))
		m_alife_simulator				= xr_new<CALifeSimulator>(&server(),&options);
}

void	game_sv_Single::OnCreate		(u16 id_who)
{
	if (!ai().get_alife())
		return;

	CSE_Abstract			*e_who			= get_entity_from_eid(id_who);
	VERIFY					(e_who);
	if (!e_who->m_bALifeControl)
		return;

	CSE_ALifeObject			*alife_object	= smart_cast<CSE_ALifeObject*>(e_who);
	if (!alife_object)
		return;

	alife_object->m_bOnline	= true;

	if (alife_object->ID_Parent != 0xffff) {
		CSE_ALifeDynamicObject			*parent = ai().alife().objects().object(alife_object->ID_Parent,true);
		if (parent) {
			CSE_ALifeTraderAbstract		*trader = smart_cast<CSE_ALifeTraderAbstract*>(parent);
			if (trader)
				alife().create			(alife_object);
			else
				alife_object->m_bALifeControl	= false;
		}
		else
			alife_object->m_bALifeControl		= false;
	}
	else
		alife().create					(alife_object);
}

shared_str game_sv_Single::level_name			(const shared_str &server_options) const
{
	if (!ai().get_alife())
		return				(inherited::level_name(server_options));
	return					(alife().level_name());
}

void game_sv_Single::restart_simulator			(LPCSTR saved_game_name)
{
	shared_str				&options = *alife().server_command_line();

	delete_data				(m_alife_simulator);
	server().clear_ids		();

	xr_strcpy					(g_pGamePersistent->m_game_params.m_game_or_spawn,saved_game_name);
	xr_strcpy					(g_pGamePersistent->m_game_params.m_new_or_load,"load");

	pApp->ls_header[0] = '\0';
	pApp->ls_tip_number[0] = '\0';
	pApp->ls_tip[0] = '\0';
	pApp->LoadBegin			();
	m_alife_simulator		= xr_new<CALifeSimulator>(&server(),&options);
//	g_pGamePersistent->LoadTitle		("st_client_synchronising");
	g_pGamePersistent->LoadTitle();
	Device.PreCache			(60, true, true);
	pApp->LoadEnd			();
}