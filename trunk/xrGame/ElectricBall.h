///////////////////////////////////////////////////////////////
// ElectricBall.h
// ElectricBall - артефакт электрический шар
///////////////////////////////////////////////////////////////

#pragma once
#include "artifact.h"

class CElectricBall : public CArtefact 
{
private:
	typedef CArtefact inherited;
public:
	CElectricBall(void);
	virtual ~CElectricBall(void);

	virtual void Load				(LPCSTR section);
	virtual void	Hide() {};
	virtual void	Show() {};

protected:
	virtual void	UpdateCLChild	();

};