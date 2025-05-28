//////////////////////////////////////////////////////////////////////
// RocketLauncher.cpp:	интерфейс дл€ семейства объектов 
//						стрел€ющих гранатами и ракетами
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RocketLauncher.h"
#include "CustomRocket.h"
#include "level.h"

CRocketLauncher::CRocketLauncher()
{
//	m_pRocket =  NULL;
}
CRocketLauncher::~CRocketLauncher()
{
}
void  CRocketLauncher::Load	(LPCSTR section)
{
	m_fLaunchSpeed = pSettings->r_float(section, "launch_speed");
}

void CRocketLauncher::AttachRocket(CGameObject* rocket, CGameObject* parent_rocket_launcher)
{
	CCustomRocket* pRocket = static_cast<CCustomRocket*>(rocket);
	pRocket->m_pOwner = static_cast<CGameObject*>(parent_rocket_launcher->H_Root());
	VERIFY(pRocket->m_pOwner);
	pRocket->H_SetParent(parent_rocket_launcher);
	m_rockets.push_back(pRocket);
}

void CRocketLauncher::DetachRocket(CGameObject* rocket, bool bLaunch)
{
	CCustomRocket *pRocket = static_cast<CCustomRocket*>(rocket);
	VERIFY(pRocket);

	ROCKETIT It = std::find(m_rockets.begin(), m_rockets.end(),pRocket);

	ROCKETIT It_l = m_launched_rockets.end();
	if (It == m_rockets.end())
		It_l = std::find(m_launched_rockets.begin(), m_launched_rockets.end(), pRocket);

	VERIFY(It != m_rockets.end()|| It_l != m_launched_rockets.end());

	if( It != m_rockets.end() )
	{
		(*It)->m_bLaunched	= bLaunch;
		(*It)->H_SetParent(NULL);
		m_rockets.erase		(It);
	};

	if( It_l != m_launched_rockets.end() )
	{
		(*It_l)->m_bLaunched			= bLaunch;
		(*It_l)->H_SetParent		(NULL);
		m_launched_rockets.erase	(It_l);
	}
}




void CRocketLauncher::LaunchRocket(const Fmatrix& xform,  
								   const Fvector& vel, 
								   const Fvector& angular_vel)
{
/*	VERIFY(m_pRocket != NULL);
	m_pRocket->SetLaunchParams(xform, vel, angular_vel);
	m_pRocket->H_SetParent(NULL);
*/
	VERIFY2(_valid(xform),"CRocketLauncher::LaunchRocket. Invalid xform argument!");
	getCurrentRocket()->SetLaunchParams(xform, vel, angular_vel);
//	Msg("---------Launched rocket [%d] frame [%d]",getCurrentRocket()->ID(), Device.dwFrame);
//	getCurrentRocket()->H_SetParent(NULL);
	m_launched_rockets.push_back( getCurrentRocket() );
	//m_rockets.pop_back();
}

CCustomRocket*	CRocketLauncher::getCurrentRocket()
{
	if( m_rockets.size() )
		return m_rockets.back();
	else
		return (CCustomRocket*)0;
}
u32				CRocketLauncher::getRocketCount()
{
	return m_rockets.size();
}
