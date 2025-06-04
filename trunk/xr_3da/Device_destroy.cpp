#include "stdafx.h"

#include "render.h"
#include "IGame_Persistent.h"
#include "../include/xrRender/RenderFactory.h"
#include "../Include/xrRender/DrawUtils.h"

void CRenderDevice::_Destroy	(BOOL bKeepTextures)
{
	DU->OnDeviceDestroy();

	// before destroy
	b_is_Ready					= FALSE;
	Statistic->OnDeviceDestroy	();
	::Render->destroy			();
	m_pRender->OnDeviceDestroy(bKeepTextures);

	Memory.mem_compact			();
}

void CRenderDevice::Destroy() {
	if (!b_is_Ready)			return;

	Log("@ Destroying Direct3D...");

	m_pRender->ValidateHW();

	_Destroy					(FALSE);

	// real destroy
	m_pRender->DestroyHW();

	DestroyWindow();

	seqRender.R.clear			();
	seqAppActivate.R.clear		();
	seqAppDeactivate.R.clear	();
	seqAppStart.R.clear			();
	seqAppEnd.R.clear			();
	seqFrame. R.clear			();
	seqFrameMT.R.clear			();
	seqDeviceReset.R.clear		();
	seqParallel.clear			();

	RenderFactory->DestroyRenderDeviceRender(m_pRender);
	m_pRender = 0;
	xr_delete					(Statistic);
}

#include "IGame_Level.h"
#include "CustomHUD.h"
void CRenderDevice::Reset		(bool precache)
{
	u32 tm_start			= TimerAsync();

	m_pRender->Reset();

	if (g_pGamePersistent)
	{
//.		g_pGamePersistent->Environment().OnDeviceCreate();
		//bNeed_re_create_env = TRUE;
		g_pGamePersistent->Environment().bNeed_re_create_env = TRUE;
	}
	_SetupStates			();
	if (precache)
		PreCache			(20, false, false);
	u32 tm_end				= TimerAsync();
	Msg						("*** RESET [%d ms]",tm_end-tm_start);
		
	seqDeviceReset.Process(rp_DeviceReset);
}
