///////////////////////////////////////////////////////////////
// BottleItem.cpp
// BottleItem - ������� � ��������, ������� ����� �������
///////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BottleItem.h"
#include "xrmessages.h"
#include "../../xrNetServer/net_utils.h"
#include "entity_alive.h"
#include "EntityCondition.h"

#define BREAK_POWER 5.f

CBottleItem::CBottleItem(void) 
{
}

CBottleItem::~CBottleItem(void) 
{
	sndBreaking.destroy();
}


void CBottleItem::Load(LPCSTR section) 
{
	inherited::Load(section);

	if(pSettings->line_exist(section, "break_particles"))
		m_sBreakParticles = pSettings->r_string(section, "break_particles");

	if(pSettings->line_exist(section, "break_sound"))
		sndBreaking.create(pSettings->r_string(section, "break_sound"),st_Effect,sg_SourceType);

	m_alcohol = READ_IF_EXISTS(pSettings, r_float, section, "eat_alcohol", 0.0f);
}

void CBottleItem::BreakToPieces()
{
	//������ ����
	sndBreaking.play_at_pos(0, Position(), false);

	//�������� �������� ����������
	if(*m_sBreakParticles)
	{
		//���������� �������
		CParticlesObject* pStaticPG; 
		pStaticPG = CParticlesObject::Create(*m_sBreakParticles,TRUE); 
		pStaticPG->play_at_pos(Position());
	}

	//������������� ��� ������ 
	DestroyObject();
}

void CBottleItem::Hit(SHit* pHDS)
{
	inherited::Hit(pHDS);

	if (pHDS->damage() > BREAK_POWER)
		BreakToPieces();
}

void CBottleItem::UseBy				(CEntityAlive* entity_alive)
{
	inherited::UseBy					(entity_alive);

	entity_alive->conditions().ChangeAlcohol(m_alcohol);
}