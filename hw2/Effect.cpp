#include "stdafx.h"
#include "Effect.h"

void Effect::Update(float fElapsedTime)
{
	m_fElapsedtime += fElapsedTime;
}

bool Effect::IsEnd() const
{
	return m_bLoop ? false : m_fElapsedtime >= m_fTotalLifetime;
}

void ExplosionEffect::Create(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, int nParticles)
{
	std::vector<ParticleVertexType> vertices(nParticles);
	for (int i = 0; i < vertices.size(); ++i) {
		vertices[i].xmf3Position = XMFLOAT3(0.f, 0.f, 0.f);
		vertices[i].xmf4Color = XMFLOAT4(1.f, 0.f, 0.f, 0.f);
		vertices[i].xmf3InitialVelocity.x = RandomGenerator::GenerateRandomFloatInRange(-3.f, 3.f);
		vertices[i].xmf3InitialVelocity.y = RandomGenerator::GenerateRandomFloatInRange(-3.f, 3.f);
		vertices[i].xmf3InitialVelocity.z = RandomGenerator::GenerateRandomFloatInRange(-3.f, 3.f);

		vertices[i].xmf2InitialSize.x = vertices[i].xmf2InitialSize.y = RandomGenerator::GenerateRandomFloatInRange(1.f, 10.f);
		vertices[i].fRandomValue = RandomGenerator::GenerateRandomFloatInRange(0.f, 1.f);
		vertices[i].fStartTime = 0.f;
		vertices[i].fLifeTime = RandomGenerator::GenerateRandomFloatInRange(0.5f, 3.f);
		vertices[i].fMass = RandomGenerator::GenerateRandomFloatInRange(1.f, 2.f);

		m_fTotalLifetime = std::max(m_fTotalLifetime, vertices[i].fLifeTime);
	}

	m_pd3dParticleBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, vertices.data(), vertices.size() * sizeof(ParticleVertexType),
		D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_pd3dParticleUploadBuffer.GetAddressOf());
	d3dVertexBufferView.BufferLocation = m_pd3dParticleBuffer->GetGPUVirtualAddress();
	d3dVertexBufferView.StrideInBytes = sizeof(ParticleVertexType);
	d3dVertexBufferView.SizeInBytes = sizeof(ParticleVertexType) * nParticles;

}

D3D12_RASTERIZER_DESC Effect::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC desc{};
	{
		desc.FillMode = D3D12_FILL_MODE_SOLID;
		desc.CullMode = D3D12_CULL_MODE_BACK;
		desc.FrontCounterClockwise = FALSE;
		desc.DepthBias = 0;
		desc.DepthBiasClamp = 0.0f;
		desc.SlopeScaledDepthBias = 0.0f;
		desc.DepthClipEnable = TRUE;
		desc.MultisampleEnable = FALSE;
		desc.AntialiasedLineEnable = FALSE;
		desc.ForcedSampleCount = 0;
		desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	}

	return desc;
}

D3D12_BLEND_DESC Effect::CreateBlendState()
{
	D3D12_BLEND_DESC desc{};
	{
		desc.AlphaToCoverageEnable = FALSE;
		desc.IndependentBlendEnable = FALSE;
		desc.RenderTarget[0].BlendEnable = FALSE;
		desc.RenderTarget[0].LogicOpEnable = FALSE;
		desc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		desc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		desc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		desc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	}

	return desc;
}

D3D12_DEPTH_STENCIL_DESC Effect::CreateDepthStencilState()
{
	D3D12_DEPTH_STENCIL_DESC desc{};
	{
		desc.DepthEnable = TRUE;
		desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		desc.StencilEnable = FALSE;
		desc.StencilReadMask = 0x00;
		desc.StencilWriteMask = 0x00;
		desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
		desc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		desc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		desc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		desc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NEVER;
	}

	return desc;
}


