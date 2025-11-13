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
	void Update(float fElapsedTime);
	void Render(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);

	bool IsEnd() const;

protected:
	void CreatePipelineState(ComPtr<ID3D12RootSignature> pd3dRootSignature = nullptr);

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreateGeometryShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

protected:
	ComPtr<ID3D12Resource> m_pd3dParticleBuffer;
	ComPtr<ID3D12Resource> m_pd3dParticleUploadBuffer;

	UINT nParticles = 0;
	D3D12_VERTEX_BUFFER_VIEW d3dVertexBufferView;

	float m_fTotalLifetime = 0.f;
	float m_fElapsedtime = 0.f;

	bool m_bLoop = false;

};

class ExplosionEffect : public Effect {
public:
	ExplosionEffect() {}
	virtual void Create(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, int nParticles) override;

protected:
	void CreatePipelineState(ComPtr<ID3D12RootSignature> pd3dRootSignature = nullptr)

private:


};