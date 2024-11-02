///////////////////////////////////////////////////////////////
// FadedBall.h
// FadedBall - артефакт блеклый шар
///////////////////////////////////////////////////////////////

#pragma once
#include "artifact.h"

class CFadedBall : public CArtefact 
{
private:
	typedef CArtefact inherited;
public:
	CFadedBall(void);
	virtual ~CFadedBall(void);

	virtual void Load				(LPCSTR section);
	virtual void	Hide() {};
	virtual void	Show() {};

protected:
};