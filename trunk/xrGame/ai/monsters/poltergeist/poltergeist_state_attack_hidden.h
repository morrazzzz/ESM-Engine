#pragma once

#include "../state.h"

template<typename _Object>
class	CStatePoltergeistAttackHidden : public CState<_Object> {
protected:
	typedef CState<_Object>		inherited;
	typedef CState<_Object>*	state_ptr;
	using inherited::object;
	using inherited::prev_substate;
	using inherited::current_substate;
	using inherited::select_state;
	using inherited::get_state_current;
	using inherited::get_state;
	using inherited::add_state;

	struct {
		Fvector point;
		u32		node;
	} m_target;


public:
					CStatePoltergeistAttackHidden	(_Object *obj) : inherited(obj) {}
	virtual			~CStatePoltergeistAttackHidden	() {}


	virtual void	initialize				();
	virtual void	execute					();

private:

			void	select_target_point		();
};

#include "poltergeist_state_attack_hidden_inline.h"
