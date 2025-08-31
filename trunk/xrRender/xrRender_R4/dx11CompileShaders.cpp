#include "stdafx.h"
#include "../xrRender/D3DCompileShaders.h"
#include "../xrRender/dxRenderDeviceRender.h"

HRESULT CD3DCompileShaders::SetDeviceShader(const char* pTarget, const void* fileData, size_t fileSize, void*& result)
{
	u32 typeShader = 0;

	HRESULT hrResult = E_FAIL;

	switch (pTarget[0])
	{
	case 'p':
	{
		SPS* ps_struct = static_cast<SPS*>(result);
		typeShader = RC_dest_pixel;
		hrResult = HW.pDevice->CreatePixelShader(fileData, fileSize, 0, &ps_struct->ps);
		break;
	}
	case 'v':
	{
		SVS* vs_struct = static_cast<SVS*>(result);
		typeShader = RC_dest_vertex;
		hrResult = HW.pDevice->CreateVertexShader(fileData, fileSize, 0, &vs_struct->vs);
		ID3DBlob* pSignatureBlob = nullptr;
		CHK_DX(D3DGetInputSignatureBlob(fileData, fileSize, &pSignatureBlob));
		R_ASSERT(pSignatureBlob);

		vs_struct->signature = dxRenderDeviceRender::Instance().Resources->_CreateInputSignature(pSignatureBlob);

		pSignatureBlob->Release();
		break;
	}
	case 'g':
	{
		SGS* gs_struct = static_cast<SGS*>(result);
		typeShader = RC_dest_geometry;
		hrResult = HW.pDevice->CreateGeometryShader(fileData, fileSize, 0, &gs_struct->gs);
		break;
	}
	case 'h':
	{
		SHS* hs_struct = static_cast<SHS*>(result);
		typeShader = RC_dest_hull;
		hrResult = HW.pDevice->CreateHullShader(fileData, fileSize, 0, &hs_struct->sh);
		break;
	}
	case 'd':
	{
		SDS* ds_struct = static_cast<SDS*>(result);
		typeShader = RC_dest_domain;
		hrResult = HW.pDevice->CreateDomainShader(fileData, fileSize, 0, &ds_struct->sh);
		break;
	}
	case 'c':
	{
		SCS* cs_struct = static_cast<SCS*>(result);
		typeShader = RC_dest_compute;
		hrResult = HW.pDevice->CreateComputeShader(fileData, fileSize, 0, &cs_struct->sh);
		break;
	}
	}

	ID3DShaderReflection* pReflection = nullptr;
	hrResult = D3DReflect(fileData, fileSize, IID_ID3DShaderReflection, (void**)&pReflection);

	if (SUCCEEDED(hrResult) && pReflection)
	{
		D3D_SHADER_DESC	ShaderDesc;
		pReflection->GetDesc(&ShaderDesc);

		otherForShaders* otherForShader = static_cast<otherForShaders*>(result);
		if (ShaderDesc.ConstantBuffers)
			otherForShader->constants.parseConstantsShader(pReflection, ShaderDesc.ConstantBuffers, typeShader);

		if (ShaderDesc.BoundResources)
			otherForShader->shaderResources.parseShaderResource(pReflection, ShaderDesc.BoundResources, typeShader);
	}

	return hrResult;
}