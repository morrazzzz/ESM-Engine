#include "pch_script.h"
#include "script_crandom.h"

using namespace luabind;

s32 MaxI()
{
	return Random.maxI();
}

s32 RandIMax(s32 max)
{
	return Random.randI(max);
}

s32 RandIMinMax(s32 min, s32 max)
{
	return Random.randI(min, max);
}

float MaxF()
{
	return Random.maxF();
}

float RandFMax(float max)
{
	return Random.randF(max);
}

float RandFMinMax(float min, float max)
{
	return Random.randF(min, max);
}

#pragma optimize("s",on)
void CScriptRandom::script_register(lua_State* L)
{
	module(L, "random")
		[
				def("maxI", &MaxI),
				def("randI", &RandIMax),
				def("randI", &RandIMinMax),
				def("maxF", &MaxF),
				def("randF", &RandFMax),
				def("randF", &RandFMinMax)
		];
}