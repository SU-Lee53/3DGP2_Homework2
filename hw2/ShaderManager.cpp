#include "stdafx.h"
#include "ShaderManager.h"

ShaderManager::ShaderManager(ComPtr<ID3D12Device> pDevice)
{
	m_pd3dDevice = pDevice;
}

ShaderManager::~ShaderManager()
{
}

void ShaderManager::Initialize()
{
	Load<StandardShader>();
	Load<TerrainShader>();
	Load<OBBDebugShader>();
	Load<MirrorShader>();
}
