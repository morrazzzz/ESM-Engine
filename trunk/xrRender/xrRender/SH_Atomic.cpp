#include "stdafx.h"
#pragma hdrstop

#include "sh_atomic.h"
#include "ResourceManager.h"
#include "dxRenderDeviceRender.h"

// Atomic
SVS::~SVS								()			{	_RELEASE(vs);		DEV->_DeleteVS			(this);	}
SPS::~SPS								()			{	_RELEASE(ps);		DEV->_DeletePS			(this);	}
SState::~SState							()			{	_RELEASE(state);	DEV->_DeleteState		(this);	}
SDeclaration::~SDeclaration				()			{	_RELEASE(dcl);		DEV->_DeleteDecl		(this);	}
