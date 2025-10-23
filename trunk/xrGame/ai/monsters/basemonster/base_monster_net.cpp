#include "stdafx.h"
#include "base_monster.h"

#include "../../../ai_object_location.h"
#include "../../../game_graph.h"
#include "../../../net_utils.h"
#include "ai_space.h"
#include "../../../CharacterPhysicsSupport.h"
#include "xrServer_Objects_ALife_Monsters.h"

BOOL CBaseMonster::net_SaveRelevant	()
{
	return (inherited::net_SaveRelevant() || BOOL(PPhysicsShell()!=NULL));
}

void CBaseMonster::SaveCSEObj(CSE_Abstract* data, bool needSaveAll)
{
	CSE_ALifeMonsterAbstract* this_object = data->cast_monster_abstract();

	this_object->fHealth = GetfHealth();
	this_object->timestamp = Device.dwTimeGlobal;
	R_ASSERT(!NET.empty());
	net_update& N = NET.back();

	this_object->o_Position = N.p_pos;
	this_object->o_model = N.o_model;
	this_object->o_torso.yaw = N.o_torso.yaw;
	this_object->o_torso.pitch = N.o_torso.pitch;
	this_object->o_torso.roll = N.o_torso.roll;
	this_object->s_team = g_Team();
	this_object->s_squad = g_Squad();
	this_object->s_group = g_Group();

	float game_vertex_id = ai_location().game_vertex_id();

	this_object->m_tNextGraphID = game_vertex_id;
	this_object->m_tPrevGraphID = game_vertex_id;

	float Points = 0;
	if (ai().game_graph().valid_vertex_id(game_vertex_id)) {
		Points = Position().distance_to(ai().game_graph().vertex(game_vertex_id)->level_point());
	}

	this_object->m_fDistanceFromPoint = Points;
	this_object->m_fDistanceToPoint = Points;

	if (!needSaveAll || !net_SaveRelevant())
		return;

	inherited::SaveCSEObj(data, needSaveAll);
	m_pPhysics_support->SaveStateSkeleton(data);
}
