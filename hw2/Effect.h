#pragma once

struct CB_PARTICLE_DATA {
	XMFLOAT3 xmf3Position;
	XMFLOAT3 xmf3Force;
	float fTotalTime;
	float m_fTotalLifetime;
};

struct ParticleVertexType {
	XMFLOAT3 xmf3Position;
	XMFLOAT4 xmf4Color;
	XMFLOAT3 xmf3InitialVelocity;
	XMFLOAT2 xmf2InitialSize;
	float fRandomValue;
	float fStartTime;
	float fLifeTime;
	float fMass;
};

class Effect {
public:
	Effect() {}

	virtual void Create(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, int nParticles) {}

protected:
	ComPtr<ID3D12Resource> m_pd3dParticleBuffer;
	ComPtr<ID3D12Resource> m_pd3dParticleUploadBuffer;

	UINT nParticles = 0;
	D3D12_VERTEX_BUFFER_VIEW d3dVertexBufferView;

	float m_fTotalLifetime = 0.f;

};

class ExplosionEffect : public Effect {
public:
	ExplosionEffect() {}
	virtual void Create(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, int nParticles) override;

private:


};