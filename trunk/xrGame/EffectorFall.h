#pragma once

#include "../xr_3da/Effector.h"

// приседание после падения
class CEffectorFall : public CEffectorCam
{
	float	fPower;
	float	fPhase;
public:
	virtual BOOL	ProcessCam		(SCamEffectorInfo& info);

	CEffectorFall(float power, float life_time=1);
	virtual ~CEffectorFall();
};

class CEffectorDOF : public CEffectorCam
{
	float			m_fPhase;
public:
	virtual BOOL	ProcessCam		(SCamEffectorInfo& info);

	CEffectorDOF(const Fvector4& dof);
};