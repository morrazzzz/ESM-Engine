#include "stdafx.h"
#include "../xrRender/du_sphere_part.h"
#include "../xrRender/du_cone.h"
#include "../xrRender/du_sphere.h"

void CRenderTarget::draw_volume(ShaderElement* shaderElement, light* L)
{
	switch(L->flags.type) {
	case IRender_Light::REFLECTED	:
	case IRender_Light::POINT		:
		if (shaderElement)
			RCache.set_Element(shaderElement, 0, &*g_accum_point);

		RCache.set_Geometry(g_accum_point);
		RCache.Render(D3DPT_TRIANGLELIST, 0, 0, DU_SPHERE_NUMVERTEX, 0, DU_SPHERE_NUMFACES);
		break;
	case IRender_Light::SPOT		:
		if (shaderElement)
			RCache.set_Element(shaderElement, 0, &*g_accum_spot);

		RCache.set_Geometry(g_accum_spot);
		RCache.Render(D3DPT_TRIANGLELIST, 0, 0, DU_CONE_NUMVERTEX, 0, DU_CONE_NUMFACES);
		break;
	case IRender_Light::OMNIPART	:
		if (shaderElement)
			RCache.set_Element(shaderElement, 0, &*g_accum_omnipart);

		RCache.set_Geometry(g_accum_omnipart);
		RCache.Render(D3DPT_TRIANGLELIST, 0, 0, DU_SPHERE_PART_NUMVERTEX, 0, DU_SPHERE_PART_NUMFACES);
		break;
	default:
		break;
	}
}
