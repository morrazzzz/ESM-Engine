#include "stdafx.h"
#include "GetRank.h"

#pragma todo("figure it out with get_rank function. MP DELETE TODO!!!!")
shared_str	g_ranks[5];

u32 get_rank(const shared_str& section)
{
	int res = -1;
	if (g_ranks[0].size() == 0)
	{ //load
		string32			buff;
		for (int i = 0; i < 5; i++)
		{
			sprintf_s(buff, "rank_%d", i);
			g_ranks[i] = pSettings->r_string(buff, "available_items");
		}
	}
	for (u32 i = 0; i < 5; i++)
	{
		if (strstr(g_ranks[i].c_str(), section.c_str()))
		{
			res = i;
			break;
		}
	}

	if (res == u32(-1))
	{
		Msg("!~! Can`t find rank for [%s]!!!!", section.c_str());
		return -1;
	}

	//R_ASSERT3	(res!=-1,"cannot find rank for", section.c_str());
	return		res;
}
