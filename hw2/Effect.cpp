#include "stdafx.h"
#include "Effect.h"

void Effect::Update(float fElapsedTime)
{
	m_fElapsedtime += fElapsedTime;
}

void Effect::Render(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{

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
