#include "stdafx.h"
#include "../xrRender/D3DCompileShaders.h"

HRESULT CD3DCompileShaders::SetDeviceShader(const char* pTarget, const void* fileData, size_t fileSize, void*& result)
{
	const void* data = nullptr;
	HRESULT hrResult = E_FAIL;
	
	u32 typeShader = 0;
	const DWORD* dwordFileData = static_cast<const DWORD*>(fileData);

	switch (pTarget[0])
	{
	case 'p':
	{
		SPS* sps_result = static_cast<SPS*>(result);
		hrResult = HW.pDevice->CreatePixelShader(dwordFileData, &sps_result->ps);
		typeShader = RC_dest_pixel;

		if (!SUCCEEDED(hrResult))
			return hrResult;

		break;
	}
	case 'v':
	{
		SVS* sps_result = static_cast<SVS*>(result);
		hrResult = HW.pDevice->CreateVertexShader(dwordFileData, &sps_result->vs);
		typeShader = RC_dest_vertex;

		if (!SUCCEEDED(hrResult))
			return hrResult;

		break;
	}
	}

	hrResult = D3DXFindShaderComment(dwordFileData, MAKEFOURCC('C', 'T', 'A', 'B'), &data, NULL);
	
	if (!SUCCEEDED(hrResult) || !data)
		return hrResult;

	LPD3DXSHADER_CONSTANTTABLE	pConstants = LPD3DXSHADER_CONSTANTTABLE(data);

	otherForShaders* constantsShader = static_cast<otherForShaders*>(result);
	constantsShader->constants.parse(pConstants, typeShader);
}