#pragma once
#include "../states/monster_state_attack.h"

template<typename _Object>
class	CBloodsuckerStateAttack : public CStateMonsterAttack<_Object> {
	typedef CStateMonsterAttack<_Object> inherited_attack;

	using inherited_attack::object;
	using inherited_attack::current_substate;
	using inherited_attack::select_state;
	using inherited_attack::inherited;
	using inherited_attack::state_ptr;
	using inherited_attack::get_state_current;
	using inherited_attack::prev_substate;

	u32				m_time_stop_invis;
	Fvector			m_dir_point;

public:
					CBloodsuckerStateAttack		(_Object *obj);
	virtual			~CBloodsuckerStateAttack	();

	virtual	void	initialize					();
	virtual	void	execute						();
	virtual	void	finalize					();
	virtual	void	critical_finalize			();
	
	virtual void	setup_substates				();
private:
			void	update_invisibility			();
			bool	check_hiding				();
};

#include "bloodsucker_attack_state_inline.h"
