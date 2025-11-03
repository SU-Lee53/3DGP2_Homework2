#include "stdafx.h"
#include "Sprite.h"
#include "Texture.h"

Sprite::Sprite(float fLeft, float fTop, float fRight, float fBottom, UINT uiLayerIndex, bool bClickable)
{
	m_Rect.fLeft = fLeft;
	m_Rect.fTop = fTop;
	m_Rect.fRight = fRight;
	m_Rect.fBottom = fBottom;

	m_bClickable = bClickable;

	m_nLayerIndex = uiLayerIndex;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TexturedSprite

TexturedSprite::TexturedSprite(const std::string& strTextureName, float fLeft, float fTop, float fRight, float fBottom, UINT uiLayerIndex, bool bClickable)
	: Sprite(fLeft, fTop, fRight, fBottom, uiLayerIndex, bClickable)
{
	m_pTexture = TEXTURE->GetTexture(strTextureName);
}

void TexturedSprite::SetTexture(std::shared_ptr<Texture> pTexture)
{
	m_pTexture = pTexture;
}

void Sprite::CreateShaderVariables(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	m_SpriteCBuffer.Create(pd3dDevice, pd3dCommandList, ConstantBufferSize<CB_SPRITE_DATA>::value, true);
}

void TexturedSprite::AddToUI(UINT nLayerIndex)
{
	UI->Add(shared_from_this(), SPRITE_TYPE_TEXTURE, nLayerIndex);
}

void TexturedSprite::Render(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, DescriptorHandle& descHandle) const
{
	CB_SPRITE_DATA spriteData;
	{
		spriteData.fLeft = m_Rect.fLeft;
		spriteData.fTop = m_Rect.fTop;
		spriteData.fRight = m_Rect.fRight;
		spriteData.fBottom = m_Rect.fBottom;
	}
	m_SpriteCBuffer.UpdateData<CB_SPRITE_DATA>(&spriteData);

	pd3dDevice->CopyDescriptorsSimple(1, descHandle.cpuHandle, m_SpriteCBuffer.GetCPUDescriptorHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descHandle.cpuHandle.ptr +=  2 * GameFramework::g_uiDescriptorHandleIncrementSize;

	pd3dCommandList->SetGraphicsRootDescriptorTable(1, descHandle.gpuHandle);
	descHandle.gpuHandle.ptr += 2 * GameFramework::g_uiDescriptorHandleIncrementSize;
	
	pd3dDevice->CopyDescriptorsSimple(1, descHandle.cpuHandle, m_pTexture->GetSRVHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descHandle.cpuHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

	pd3dCommandList->SetGraphicsRootDescriptorTable(2, descHandle.gpuHandle);
	descHandle.gpuHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

	pd3dCommandList->DrawInstanced(1, 1, 0, 0);
}

void Sprite::SetLayerIndex(UINT uiLayerIndex)
{
	m_nLayerIndex = uiLayerIndex;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TextSprite

TextSprite::TextSprite(const std::string& strText, float fLeft, float fTop, float fRight, float fBottom, XMFLOAT4 xmf4TextColor, UINT uiLayerIndex, bool bClickable)
	: Sprite(fLeft, fTop, fRight, fBottom, uiLayerIndex, bClickable)
{
	assert(strText.length() < MAX_CHARACTER_PER_SPRITE);
	const char* cstrText = strText.c_str();
	strcpy_s(m_cstrText, strText.length() + 1, cstrText);	// NULL 문자 포함
	m_nTextLength = strText.length();

	m_xmf4TextColor = xmf4TextColor;
}

void TextSprite::SetText(const std::string& strText)
{
	assert(strText.length() < MAX_CHARACTER_PER_SPRITE);
	const char* cstrText = strText.c_str();
	strcpy_s(m_cstrText, strText.length() + 1, cstrText);	// NULL 문자 포함
	m_nTextLength = strText.length();
}

void TextSprite::SetTextColor(const XMFLOAT4& xmf4TextColor)
{
	m_xmf4TextColor = xmf4TextColor;
}

void TextSprite::CreateShaderVariables(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	m_TextCBuffer.Create(pd3dDevice, pd3dCommandList, ConstantBufferSize<CB_TEXT_DATA>::value, true);
	Sprite::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void TextSprite::AddToUI(UINT nLayerIndex)
{
	UI->Add(shared_from_this(), SPRITE_TYPE_TEXT, nLayerIndex);
}

void TextSprite::Render(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, DescriptorHandle& descHandle) const
{
	CB_SPRITE_DATA spriteData;
	{
		spriteData.fLeft = m_Rect.fLeft;
		spriteData.fTop = m_Rect.fTop;
		spriteData.fRight = m_Rect.fRight;
		spriteData.fBottom = m_Rect.fBottom;
	}
	m_SpriteCBuffer.UpdateData<CB_SPRITE_DATA>(&spriteData);
	
	CB_TEXT_DATA textData;
	{
		memset(textData.nCharacters, 0, sizeof(textData.nCharacters));

		for (int i = 0; i < m_nTextLength; ++i) {
			textData.nCharacters[i] = m_cstrText[i];
		}
		//textData.pad = XMUINT2(99, 99);
		textData.nLength = m_nTextLength;
		textData.xmf4TextColor = m_xmf4TextColor;
	}
	m_TextCBuffer.UpdateData<CB_TEXT_DATA>(&textData);


	pd3dDevice->CopyDescriptorsSimple(1, descHandle.cpuHandle, m_SpriteCBuffer.GetCPUDescriptorHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descHandle.cpuHandle.ptr += 1 * GameFramework::g_uiDescriptorHandleIncrementSize;

	pd3dDevice->CopyDescriptorsSimple(1, descHandle.cpuHandle, m_TextCBuffer.GetCPUDescriptorHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descHandle.cpuHandle.ptr += 1 * GameFramework::g_uiDescriptorHandleIncrementSize;

	pd3dCommandList->SetGraphicsRootDescriptorTable(1, descHandle.gpuHandle);
	descHandle.gpuHandle.ptr += 2 * GameFramework::g_uiDescriptorHandleIncrementSize;

	pd3dCommandList->DrawInstanced(1, m_nTextLength, 0, 0);
}
