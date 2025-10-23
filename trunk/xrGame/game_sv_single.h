#pragma once

#include "game_sv_base.h"

class xrServer;
class CALifeSimulator;

class	game_sv_Single				: public game_sv_GameState
{
private:
	typedef game_sv_GameState inherited;

protected:
	CALifeSimulator					*m_alife_simulator;

public:
									game_sv_Single			();
	virtual							~game_sv_Single			();

	virtual		void				Create					(shared_str& options);
//	virtual		CSE_Abstract*		get_entity_from_eid		(u16 id);


	virtual		void				OnCreate				(u16 id_who);

	// Main

	virtual		shared_str			level_name				(const shared_str &server_options) const;
				void				restart_simulator		(LPCSTR saved_game_name);

	IC			xrServer			&server					() const
	{
		VERIFY						(m_server);
		return						(*m_server);
	}

	IC			CALifeSimulator		&alife					() const
	{
		VERIFY						(m_alife_simulator);
		return						(*m_alife_simulator);
	}
};
