#pragma once
#include "Effect.h"

class EffectRenderer {
public:
	EffectRenderer() {}

public:
	void Initialize(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);
	void Update(float fTimeElapsed);
	void Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);

private:
	void CreateRootSignature();

private:
	ComPtr<ID3D12Device> m_pd3dDevice;	// Ref to GameFramework::m_pd3dDevice;
	ComPtr<ID3D12RootSignature> m_pd3dRootSignature;

	float m_fTotalTime = 0.f;

	std::vector<std::shared_ptr<Effect>> m_EffectsToRender;

	std::unordered_map<std::type_index, std::shared_ptr<Effect>> m_EffectLoaded;

};

