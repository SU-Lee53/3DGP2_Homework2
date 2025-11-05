#include "stdafx.h"
#include "Material.h"
#include "Texture.h"

//////////////
// Material //
//////////////

std::shared_ptr<Shader> Material::m_pIlluminatedShader = nullptr;

Material::Material(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	Create(pd3dDevice, pd3dCommandList);
}

void Material::Create(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	m_MaterialCBuffer.Create(pd3dDevice, pd3dCommandList, ConstantBufferSize<CB_MATERIAL_DATA>::value, true);
}
void Material::UpdateShaderVariable(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	CB_MATERIAL_DATA cbData{};
	{
		cbData.xmf4Ambient = m_xmf4AmbientColor;
		cbData.xmf4Diffuse = m_xmf4AlbedoColor;
		cbData.xmf4Specular = m_xmf4SpecularColor;
		cbData.xmf4Emissive = m_xmf4EmissiveColor;
		cbData.nMaterialType = m_nType;
	}
	m_MaterialCBuffer.UpdateData(&cbData);
}

void Material::SetMaterialToPipeline(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, UINT uiRootParameterIndex)
{
	m_MaterialCBuffer.SetBufferToPipeline(pd3dCommandList, uiRootParameterIndex);
}

void Material::OnPrepareRender(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	m_pShader->OnPrepareRender(pd3dCommandList);
}

void Material::SetTexture(UINT nTextureIndex, std::shared_ptr<Texture> pTexture)
{
	m_pTextures[nTextureIndex] = pTexture;
}

void Material::PrepareShaders(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12RootSignature> pd3dRootSignature)
{
	m_pIlluminatedShader = std::make_shared<IlluminatedShader>();
	m_pIlluminatedShader->Create(pd3dDevice, pd3dRootSignature);
}
