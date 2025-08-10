#include "stdafx.h"
#include "dx11ShaderResources.h"

void dx11ShaderResources::addShaderResource(LPCSTR name, u32 index)
{
	auto it = mapResourcesShader.find(name);
	VERIFY(it == mapResourcesShader.end());
	
	xr_string string = name;
	mapResourcesShader[string] = index;
}

void dx11ShaderResources::parseShaderResource(ID3DShaderReflection* pReflection, u32 resNum, u32 destination)
{
	for (u32 i = 0; i < resNum; i++)
	{
		D3D_SHADER_INPUT_BIND_DESC	ResDesc;
		pReflection->GetResourceBindingDesc(i, &ResDesc);

		u16	type = 0;

		switch (ResDesc.Type)
		{
		case D3D10_SIT_TEXTURE:
			type = RC_dx10texture;
			break;
		case D3D10_SIT_SAMPLER:
			type = RC_sampler;
			break;
		case D3D11_SIT_UAV_RWTYPED:
			type = RC_dx11UAV;
			break;
		default:
//			R_ASSERT(!"NoDefault in dx11ShaderResources!!!");
//			return;
			continue;
		}

		VERIFY(ResDesc.BindCount == 1);

		u16	r_index = static_cast<u16>(-1);

		if (destination & RC_dest_pixel)
		{
			r_index = u16(ResDesc.BindPoint + CTexture::rstPixel);
		}
		else if (destination & RC_dest_vertex)
		{
			r_index = u16(ResDesc.BindPoint + CTexture::rstVertex);
		}
		else if (destination & RC_dest_geometry)
		{
			r_index = u16(ResDesc.BindPoint + CTexture::rstGeometry);
		}
		else if (destination & RC_dest_hull)
		{
			r_index = u16(ResDesc.BindPoint + CTexture::rstHull);
		}
		else if (destination & RC_dest_domain)
		{
			r_index = u16(ResDesc.BindPoint + CTexture::rstDomain);
		}
		else if (destination & RC_dest_compute)
		{
			r_index = u16(ResDesc.BindPoint + CTexture::rstCompute);
		}
		R_ASSERT(r_index != static_cast<u16>(-1));
		
		addShaderResource(ResDesc.Name, r_index);
	}
}

u32 dx11ShaderResources::findResourceShader(LPCSTR name)
{
	auto it = mapResourcesShader.find(name);
	if (it == mapResourcesShader.end())
		return static_cast<u32>(-1);
	
	return it->second;
}

void dx11ShaderResources::mergeShaderResouces(std::initializer_list<dx11ShaderResources*> dx11ShaderResourcesList)
{
	R_ASSERT(mapResourcesShader.empty());
	for (const auto& list : dx11ShaderResourcesList)
	{
		for (auto& map : list->mapResourcesShader)
			mapResourcesShader.insert(std::move(map));
	}
}

/*
dx11ShaderResources& dx11ShaderResources::operator=(dx11ShaderResources&& oldShaderResources)
{
	mapResourcesShader = std::move(oldShaderResources.mapResourcesShader);
	return *this;
}
*/