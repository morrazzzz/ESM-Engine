///////////////////////////////////////////////////////////////
// DummyArtifact.h
// DummyArtefact - артефакт пустышка
///////////////////////////////////////////////////////////////

#pragma once
#include "artifact.h"

class CDummyArtefact : public CArtefact 
{
private:
	typedef CArtefact inherited;
public:
	CDummyArtefact(void);
	virtual ~CDummyArtefact(void);

	virtual void Load				(LPCSTR section);
	virtual void	Hide() {};
	virtual void	Show() {};

protected:
};