#pragma once

#include "../../xrNetServer/net_server.h"
#include "game_sv_base.h"
#include "id_generator.h"

class CSE_Abstract;

const u32	NET_Latency		= 50;		// time in (ms)

// t-defs
typedef xr_unordered_map<u32,CSE_Abstract*>	xrS_entities;

// main

class xrServer	: public IPureServer  
{
	xrS_entities entities;
	xr_vector<CSE_Abstract*> EntitiesToSpawn{};

	typedef 
		CID_Generator<
			u32,		// time identifier type
			u8,			// compressed id type 
			u16,		// id type
			u8,			// block id type
			u16,		// chunk id type
			0,			// min value
			u16(-2),	// max value
			256,		// block size
			u16(-1)		// invalid id
		> id_generator_type;

	id_generator_type		m_tID_Generator;
public:
	game_sv_GameState*		game;

	IC void					clear_ids				()
	{
		m_tID_Generator		= id_generator_type();
	}
	IC u16					PerformIDgen			(u16 ID)
	{
		return				(m_tID_Generator.tfGetID(ID));
	}
	IC void					FreeID					(u16 ID, u32 time)
	{
		return				(m_tID_Generator.vfFreeID(ID, time));
	}

	CSE_Abstract*			Process_spawn			(NET_Packet& P, CSE_Abstract* tpExistedEntity = 0);
	void					Process_save			(NET_Packet& P);
	void					Process_event			(NET_Packet& P);

	// constr / destr
	xrServer				();
	virtual ~xrServer		();

	// extended functionality
	void OnMessage(NET_Packet& P);	// Non-Zero means broadcasting with "flags" as returned
	virtual void			SendTo_LL			(void* data, u32 size);

	// utilities
	CSE_Abstract*			entity_Create		(LPCSTR name);
	void					entity_Destroy		(CSE_Abstract *&P);
	size_t						GetEntitiesNum		()			{ return entities.size(); };

	CSE_Abstract*			ID_to_entity		(u16 ID);

	// main
	void Connect(shared_str& options);
	virtual void			Disconnect			();
	virtual void			Update				();
	void					SLS_Default			();
	void					SLS_Save			(IWriter&	fs);
			shared_str		level_name			(const shared_str &server_options) const;

	void SpawnNewObjects();
	void DestroyAllEntities();
};
