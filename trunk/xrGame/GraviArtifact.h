///////////////////////////////////////////////////////////////
// GraviArtifact.h
// GraviArtefact - гравитационный артефакт, прыгает на месте
// и парит над землей
///////////////////////////////////////////////////////////////

#pragma once
#include "artifact.h"

class CGraviArtefact : public CArtefact 
{
private:
	typedef CArtefact inherited;
public:
	CGraviArtefact(void);
	virtual ~CGraviArtefact(void);

	virtual void Load				(LPCSTR section);
	virtual void	Hide() {};
	virtual void	Show() {};

protected:
	virtual void	UpdateCLChild	();
	//параметры артефакта
	float m_fJumpHeight;
	float m_fEnergy;
};
