#pragma once

class CD3DCompileShaders
{
	struct SD3DCompileShadersIncluder : public ID3DInclude
	{
		HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override;
		HRESULT __stdcall Close(LPCVOID pData) override;
	};
	
	SD3DCompileShadersIncluder D3DIncluder;

	HRESULT SetShader(const char* pTarget, const void* fileData, size_t fileSize, std::string_view fileShader, void*& result, bool const disasm);

	HRESULT SetDeviceShader(const char* pTarget, const void* fileData, size_t fileSize, void*& result);

	bool needReplacePrefix(const char* fileFormat) const;
public:
	void CompileShaders(const char* fileName, const char* fileFormat, const char* shaderEntrypoint, 
		const char* shaderTarget, bool defaultShader, u32 compilerFlags, void*& result);
};

extern CD3DCompileShaders D3DCompileShaders;

struct CRenderShadersInfo
{
	bool bFillStaticShadersMacro{ true };
	bool bFillDynamicShadersMacro{ true };

	size_t lastIndexStaticInfo = 0;
	size_t lastIndexStaticStringInfo = 0;

	xr_string shaderCacheName;
	xr_vector<D3D_SHADER_MACRO> shadersMacro{};

	inline void moveShadersMacro(xr_vector<D3D_SHADER_MACRO>& oldShadersMacro)
	{
		for (u32 i = 0; i < oldShadersMacro.size(); i++)
			shadersMacro.emplace_back(std::move(oldShadersMacro[i]));
	}

	inline void fillShaderInfoString(const char* nameDefines, char* string, bool notFillCache = false)
	{
		shadersMacro.emplace_back(D3D_SHADER_MACRO{ nameDefines, string });
		
		if (!notFillCache)
			fillShaderCacheInfoString(string);
	}

	inline void fillShaderMacroBool(const char* nameDefines, u32 value, const char* minValueDefine = "1", bool ignoreCheckValue = false)
	{
		if (ignoreCheckValue || value >= 1)
			shadersMacro.emplace_back(D3D_SHADER_MACRO{ nameDefines, minValueDefine });

		fillShaderCacheInfoBool(value);
	}

	bool needClearDynamicMacros();
private:
	inline void fillShaderCacheInfoBool(u32 value)
	{
		shaderCacheName += '0' + static_cast<char>(value);
	}

	inline void fillShaderCacheInfoString(char* string)
	{
		shaderCacheName += string;
	}

	inline bool checkSkinning(const char* nameDefine)
	{
		const char* skinningName = "";
		switch (RImplementation.m_skinning)
		{
		case -1:
			skinningName = "SKIN_NONE";
			break;
		case 0:
			skinningName = "SKIN_0";
			break;
		case 1:
			skinningName = "SKIN_1";
			break;
		case 2:
			skinningName = "SKIN_2";
			break;
		case 3:
			skinningName = "SKIN_3";
			break;
		case 4:
			skinningName = "SKIN_4";
			break;
		}

		return !xr_strcmp(skinningName, nameDefine);
	}
};

extern CRenderShadersInfo renderShadersMacro;