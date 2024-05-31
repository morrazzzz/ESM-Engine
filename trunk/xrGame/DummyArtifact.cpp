///////////////////////////////////////////////////////////////
// DummyArtifact.cpp
// DummyArtefact - артефакт пустышка
///////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DummyArtifact.h"
#include "../xrPhysics/PhysicsShell.h"


CDummyArtefact::CDummyArtefact() 
{
}

CDummyArtefact::~CDummyArtefact() 
{
}

void CDummyArtefact::Load(LPCSTR section) 
{
	inherited::Load(section);
}

