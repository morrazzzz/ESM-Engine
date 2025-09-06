#pragma once

//INFO: Parser samplers and textures. Not for constants!!!

class dx11ShaderResources
{
	xr_map<xr_string, u32> mapResourcesShader;

public:
	dx11ShaderResources() = default;
	~dx11ShaderResources() = default;

	void addShaderResource(LPCSTR name, u32 index);
	void mergeShaderResouces(std::initializer_list<dx11ShaderResources*> oldMapResourcesShader);
	void parseShaderResource(ID3D11ShaderReflection* pReflection, u32 ResNum, u32 destination);
	void clearShaderResources() { mapResourcesShader.clear(); }
	u32 findResourceShader(LPCSTR name);

//	dx11ShaderResources& operator=(dx11ShaderResources&& oldShaderResources);
};