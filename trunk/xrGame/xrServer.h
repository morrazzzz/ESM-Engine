#pragma once
// xrServer.h: interface for the xrServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XRSERVER_H__65728A25_16FC_4A7B_8CCE_D798CA5EC64E__INCLUDED_)
#define AFX_XRSERVER_H__65728A25_16FC_4A7B_8CCE_D798CA5EC64E__INCLUDED_
#pragma once

#include "../../xrNetServer/net_server.h"
#include "game_sv_base.h"
#include "id_generator.h"

#ifdef DEBUG
//. #define SLOW_VERIFY_ENTITIES
#endif


class CSE_Abstract;

const u32	NET_Latency		= 50;		// time in (ms)

// t-defs
typedef xr_unordered_map<u32,CSE_Abstract*>	xrS_entities;

// main

class xrServer	: public IPureServer  
{
private:
	xrS_entities				entities;
	xr_vector<CSE_Abstract*> EntitiesToSpawn{};
private:
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

private:
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

	void					Perform_reject			(CSE_Abstract* what, CSE_Abstract* from, int delta);
	void					Perform_destroy			(CSE_Abstract* tpSE_Abstract);

	CSE_Abstract*			Process_spawn			(NET_Packet& P, CSE_Abstract* tpExistedEntity = 0);
	void					Process_update			(NET_Packet& P);
	void					Process_save			(NET_Packet& P);
	void					Process_event			(NET_Packet& P);
	void					Process_event_ownership	(NET_Packet& P, u32 time, u16 ID);
	bool					Process_event_reject	(NET_Packet& P, const u32 time, const u16 id_parent, const u16 id_entity, bool send_message = true);
	void					Process_event_destroy	(NET_Packet& P, u32 time, u16 ID, NET_Packet* pEPack);

public:
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
	CSE_Abstract*			GetEntity			(u32 Num);

	CSE_Abstract*			ID_to_entity		(u16 ID);

	// main
	bool Connect(shared_str& session_name);
	virtual void			Disconnect			();
	virtual void			Update				();
	void					SLS_Default			();
	void					SLS_Clear			();
	void					SLS_Save			(IWriter&	fs);
	void					SLS_Load			(IReader&	fs);	
			shared_str		level_name			(const shared_str &server_options) const;

    void new_client();

	virtual void			Assign_ServerType	( string512& res ) {};
	virtual bool			HasPassword			()	{ return false; }
	virtual bool			HasProtected		()	{ return false; }

public:
	xr_string				ent_name_safe		(u16 eid);
#ifdef DEBUG
			bool			verify_entities		() const;
			void			verify_entity		(const CSE_Abstract *entity) const;
#endif
			void SpawnNewObjects();
};

#endif // !defined(AFX_XRSERVER_H__65728A25_16FC_4A7B_8CCE_D798CA5EC64E__INCLUDED_)
