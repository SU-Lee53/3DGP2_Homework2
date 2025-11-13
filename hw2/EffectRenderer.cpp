#include "stdafx.h"
#include "EffectRenderer.h"

void EffectRenderer::Initialize(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	m_pd3dDevice = pd3dDevice;

	std::shared_ptr<ExplosionEffect> pExplodeEffect = std::make_shared<ExplosionEffect>();
	pExplodeEffect->Create(pd3dDevice, pd3dCommandList, 10000);
}

void EffectRenderer::Update(float fTimeElapsed)
{
}

void EffectRenderer::Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	
}
