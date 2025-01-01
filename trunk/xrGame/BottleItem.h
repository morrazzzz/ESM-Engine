///////////////////////////////////////////////////////////////
// BottleItem.h
// BottleItem - ������� � ��������, ������� ����� �������
///////////////////////////////////////////////////////////////


#pragma once

#include "fooditem.h"


class CBottleItem : public CFoodItem
{
private:
	typedef	CFoodItem inherited;
public:
	CBottleItem(void);
	virtual ~CBottleItem(void);


	virtual void Load(LPCSTR section);

	virtual	void	Hit(SHit* pHDS);


	void					BreakToPieces();
	virtual void					UseBy(CEntityAlive* entity_alive);
protected:
	float		m_alcohol;
	//�������� ���������� �������
	shared_str m_sBreakParticles;
	ref_sound sndBreaking;
};