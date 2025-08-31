#include "stdafx.h"
#include "D3DCompileShaders.h"

#include <d3dcompiler.h>

CRenderShadersInfo renderShadersMacro;
CD3DCompileShaders D3DCompileShaders;

HRESULT CD3DCompileShaders::SD3DCompileShadersIncluder::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
{
	xr_string fileName = RImplementation.getShaderFolder() + "\\" + pFileName;

	IReader* file = FS.r_open("$game_shaders$", fileName.c_str());

	if (!file)
	{
		file = FS.r_open("$game_shaders$", pFileName);
		if (!file)
			return E_FAIL;
	}

	u32 size = file->length();
	u8* data = xr_alloc<u8>(size + 1);
	std::memcpy(data, file->pointer(), size);
	data[size] = 0;
	FS.r_close(file);

	*ppData = data;
	*pBytes = size;
	return S_OK;
}

HRESULT CD3DCompileShaders::SD3DCompileShadersIncluder::Close(LPCVOID pData)
{
	xr_free(pData);
	return S_OK;
}

bool CD3DCompileShaders::needReplacePrefix(const char* fileFormat) const
{
	if (!xr_strcmp(fileFormat, ".vs"))
		return RImplementation.m_skinning > -1;

	if (!xr_strcmp(fileFormat, ".ps"))
		return RImplementation.m_MSAASample > -1;
	
	return false;
}

void CD3DCompileShaders::CompileShaders(const char* fileName, const char* fileFormat,
	const char* shaderEntrypoint, const char* shaderTarget, bool defaultShader, u32 compilerFlags, void*& result)
{
	CTimer T; T.Start();

	static xr_string nameCacheFolder = "shaders_cache\\";

	xr_string shaderFile = pathShaderFolder + fileName + fileFormat;
	xr_string shaderCacheFile = nameCacheFolder + shaderFile;
	std::string_view prefixShaderCache{};

	if (needReplacePrefix(fileFormat))
	{
		size_t findPrefix = shaderFile.rfind('_');

		if (findPrefix != static_cast<size_t>(-1))
		{
			std::string_view viewShaderFile = shaderFile;
			prefixShaderCache = viewShaderFile.substr(findPrefix, 2);

			size_t offEndStringPath = pathShaderFolder.size();
			size_t sizeFileName = strlen(fileName);

			shaderFile.replace(offEndStringPath, sizeFileName, viewShaderFile.substr(offEndStringPath, sizeFileName - 2));
			
		}
	}

	string_path updatePath;

	FS.update_path(updatePath, "$game_shaders$", shaderFile.c_str());
	IReader* file = FS.r_open(updatePath);

	if (!defaultShader)
		R_ASSERT2(file, make_string("Failed compiling shader. Missing shader: [%s]", fileName));

	if (defaultShader && !file)
	{
		shaderFile.replace(pathShaderFolder.size(), strlen(fileName), "stub_default");
		file = FS.r_open(shaderFile.c_str());

		Msg("!! Missing shader: [%s][%s] in shader folder: [%s]. Using default shader: [stub_default%s]!!!",
			fileName, fileFormat, RImplementation.getShaderFolder().c_str(), fileFormat);

		R_ASSERT2(file, make_string("Missing default shader: [stub_default%s] in shader folder: [%s]!!!", fileFormat,
			RImplementation.getShaderFolder().c_str()));
	}

	const DWORD* dataFile = static_cast<const DWORD*>(file->pointer());
	UINT lengthFile = static_cast<UINT>(file->length());

	HRESULT hrResult = E_FAIL;

	if (renderShadersMacro.bFillStaticShadersMacro)
	{
		RImplementation.fillShadersStaticMacro();
		
		renderShadersMacro.bFillStaticShadersMacro = false;
		renderShadersMacro.lastIndexStaticInfo = renderShadersMacro.shadersMacro.size();
		renderShadersMacro.lastIndexStaticStringInfo = renderShadersMacro.shaderCacheName.size();
	}
	
	if (renderShadersMacro.bFillDynamicShadersMacro || renderShadersMacro.needClearDynamicMacros())
	{
		RImplementation.fillShadersDynamicMacro();
		renderShadersMacro.bFillDynamicShadersMacro = false;
	}

	string_path filePath;
	shaderCacheFile += "\\" + renderShadersMacro.shaderCacheName;
	FS.update_path(filePath, "$app_data_root$", shaderCacheFile.c_str());

	std::string_view nameFileShader = std::string_view(shaderFile).substr(pathShaderFolder.size());

	bool needCompileShader = true;
	if (FS.exist(filePath))
	{
		IReader* fileCache = FS.r_open(filePath);
		
		if (fileCache->length() > 4)
		{
			u32 crc = 0;
			crc = fileCache->r_u32();

			u32 const real_crc = crc32(fileCache->pointer(), fileCache->elapsed());

			if (real_crc == crc) {
				if (SUCCEEDED(SetShader(shaderTarget, (DWORD*)fileCache->pointer(), fileCache->elapsed(),
					nameFileShader.data(), result, RImplementation.o.disasm)))
				{
					needCompileShader = false;
				}
			}
		}

		xr_delete(fileCache);
	}

	if (needCompileShader)
	{
		LPD3DBLOB					pShaderBuf = NULL;
		LPD3DBLOB					pErrorBuf = NULL;
		hrResult =
			D3DCompile(
				dataFile,
				lengthFile,
				"",//NULL, //LPCSTR pFileName,	//	NVPerfHUD bug workaround.
				renderShadersMacro.shadersMacro.data(),
				&D3DIncluder, shaderEntrypoint,
				shaderTarget,
				compilerFlags,
				0,
				&pShaderBuf,
				&pErrorBuf
			);

		LPCSTR errorBuf = pErrorBuf ? static_cast<LPCSTR>(pErrorBuf->GetBufferPointer()) : "Empty error buffer";

		if (FAILED(hrResult))
		{
			Msg("!!! Failed compiling shader!!!");
			Msg("!!! Error buffer compiling: [%s]", errorBuf);
			R_ASSERT2(false,
				make_string("!! Failed compiling shader: [%s] with shader cache name: [%s]. See log for detail info",
					nameFileShader.data(), std::string_view(shaderCacheFile).substr(nameCacheFolder.size()).data()));
		}

		IWriter* fileWriter = FS.w_open(filePath);

		u32 const crc = crc32(pShaderBuf->GetBufferPointer(), static_cast<u32>(pShaderBuf->GetBufferSize()));

		fileWriter->w_u32(crc);
		fileWriter->w(pShaderBuf->GetBufferPointer(), (u32)pShaderBuf->GetBufferSize());
		FS.w_close(fileWriter);

		 SetShader(shaderTarget, (DWORD*)pShaderBuf->GetBufferPointer(), (u32)pShaderBuf->GetBufferSize(),
			nameFileShader.data(), result, RImplementation.o.disasm);

		R_ASSERT2(SUCCEEDED(hrResult), make_string("!! Failed SetShader: [%s][%s]", fileName, fileFormat));
	}

	Msg("Compile shader: [%f] ms", T.GetElapsed_sec() * 1000.0f);
}

HRESULT CD3DCompileShaders::SetShader(const char* pTarget, const void* fileData, size_t fileSize, std::string_view fileShader, void*& result, bool const disasm)
{
	HRESULT hr = SetDeviceShader(pTarget, fileData, fileSize, result);

	if (!SUCCEEDED(hr))
	{
		Msg("! Failed create device shader: [%s]", fileShader.data());
		R_ASSERT2(false, make_string("Failed SetShader: [%s]", fileShader.data()));
		return E_FAIL;
	}

	if (disasm)
	{
		ID3DBlob* disasmBlob = 0;
		D3DDisassemble(fileData, fileSize, 0, 0, &disasmBlob);
		//D3DXDisassembleShader		(LPDWORD(code->GetBufferPointer()), FALSE, 0, &disasm );

		static xr_string disasmFolder = "disasm\\";
		xr_string disasmFile = disasmFolder + fileShader.data();

		IWriter* W = FS.w_open("$logs$", disasmFile.c_str());
		W->w(disasmBlob->GetBufferPointer(), (u32)disasmBlob->GetBufferSize());
		FS.w_close(W);

		disasmBlob->Release();
	}
	
	return S_OK;
}

bool CRenderShadersInfo::needClearDynamicMacros()
{
	bool fillDynamicShadersMacro = false;
	shadersMacro.erase(std::remove_if(shadersMacro.begin() + lastIndexStaticInfo, shadersMacro.end(), [this, &fillDynamicShadersMacro](D3D_SHADER_MACRO macroShader)
		{
			if (!fillDynamicShadersMacro && macroShader.Name && macroShader.Definition && !checkSkinning(macroShader.Name))
			{
				fillDynamicShadersMacro = true;
				return true;
			}

			return fillDynamicShadersMacro;
		}), shadersMacro.end());


	if (fillDynamicShadersMacro)
		shaderCacheName = std::move(shaderCacheName.substr(0, lastIndexStaticStringInfo));

	return fillDynamicShadersMacro;
}