#pragma once

#include "../xr_3da/Effector.h"

// приседание после падения
class CEffectorFall : public CEffectorCam
{
	float	fPower;
	float	fPhase;
public:
	virtual	BOOL	Process(Fvector &p, Fvector &d, Fvector &n, float& fFov, float& fFar, float& fAspect);

	CEffectorFall(float power, float life_time=1);
	virtual ~CEffectorFall();
};

class CEffectorDOF : public CEffectorCam
{
	float			m_fPhase;
public:
	CEffectorDOF(const Fvector4& dof);
	virtual	BOOL	Process(Fvector& p, Fvector& d, Fvector& n, float& fFov, float& fFar, float& fAspect);
};