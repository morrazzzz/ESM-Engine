#pragma once

#include "../state.h"

template<typename _Object>
class CStateMonsterFindEnemyAngry : public CState<_Object> {
	typedef CState<_Object> inherited;
	using inherited::object;
	using inherited::time_state_started;

public:
						CStateMonsterFindEnemyAngry	(_Object *obj);
	virtual				~CStateMonsterFindEnemyAngry();

	virtual	void		execute						();
	virtual bool		check_completion			();
};

#include "monster_state_find_enemy_angry_inline.h"
