#include "stdafx.h"
#pragma hdrstop

#include "blender_nightvision.h"

CBlender_nightvision::CBlender_nightvision()
{
	description.CLS = 0;
}

CBlender_nightvision::~CBlender_nightvision()
{

}

void CBlender_nightvision::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	switch (C.iElement)
	{
	case 0:
		C.r_Pass("stub_fullscreen_triangle", "nightvision", FALSE, FALSE, FALSE);
		C.r_dx10Texture("s_image", r2_RT_generic0);
		C.r_dx10Sampler("smp_rtlinear");
		C.r_End();
		break;
	}
}