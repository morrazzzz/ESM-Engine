#include "stdafx.h"
#include "physicsshell.h"
#include "phinterpolation.h"
#include "phobject.h"
#include "phworld.h"
#include "phshell.h"

void CPHShell::net_Export(NET_Packet& P)
{
	for (auto& i : elements)
		i->net_Export(P);
}