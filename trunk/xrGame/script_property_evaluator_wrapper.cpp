////////////////////////////////////////////////////////////////////////////
//	Module 		: script_property_evaluator_wrapper.cpp
//	Created 	: 19.03.2004
//  Modified 	: 26.03.2004
//	Author		: Dmitriy Iassenev
//	Description : Script property evaluator wrapper
////////////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "script_property_evaluator_wrapper.h"
#include "script_game_object.h"
#include "ai_space.h"
#include "script_engine.h"

void CScriptPropertyEvaluatorWrapper::setup			(CScriptGameObject *object, CPropertyStorage *storage)
{
	luabind::call_member<void>	(this,"setup",object,storage);
}

void CScriptPropertyEvaluatorWrapper::setup_static	(CScriptPropertyEvaluator *evaluator, CScriptGameObject *object, CPropertyStorage *storage)
{
	evaluator->CScriptPropertyEvaluator::setup(object,storage);
}

bool CScriptPropertyEvaluatorWrapper::evaluate		()
{
	return luabind::call_member<bool>(this, "evaluate");
}

bool CScriptPropertyEvaluatorWrapper::evaluate_static	(CScriptPropertyEvaluator *evaluator)
{
	return		(evaluator->CScriptPropertyEvaluator::evaluate());
}
