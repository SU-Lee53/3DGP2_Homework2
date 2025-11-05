#pragma once
#include "GameObject.h"


class TerrainHeightMap {
public:
	TerrainHeightMap(std::string_view wsvFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale);
	~TerrainHeightMap();

	float GetHeight(float fx, float fz);
	XMFLOAT3 GetHeightMapNormal(int x, int z);

	XMFLOAT3 GetScale() { return m_xmf3Scale; }
	std::vector<BYTE>& GetHeightMapPixels() { return m_HeightMapPixels; }
	int GetHeightMapWidth() { return m_nWidth; }
	int GetHeightMapLength() { return m_nLength; }

	BYTE& operator[](size_t idx) {
		return m_HeightMapPixels[idx];
	}


private:
	std::vector<BYTE> m_HeightMapPixels = {};

	int m_nWidth = 0;
	int m_nLength = 0;

	XMFLOAT3 m_xmf3Scale = {};

};

class TerrainObject : public GameObject {
public:
	TerrainObject();
	~TerrainObject();

public:
	void Initialize(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, 
		const std::string & strFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color);

	void UpdateShaderVariables(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList);
	void Render(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, DescriptorHandle& refDescHandle);

public:
	float GetHeight(float x, float z) { return (m_pHeightMapImage->GetHeight(x / m_xmf3Scale.x, z / m_xmf3Scale.z) * m_xmf3Scale.y); }
	XMFLOAT3 GetNormal(float x, float z) { return (m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z)));  }

	int GetHeightMapWidth() { return m_pHeightMapImage->GetHeightMapWidth(); }
	int GetHeightMapLength() { return m_pHeightMapImage->GetHeightMapLength(); }

	XMFLOAT3 GetScale() { return m_xmf3Scale; }

	float GetWidth() { return m_nWidth * m_xmf3Scale.x; }
	float GetLength() { return m_nLength * m_xmf3Scale.z; }

	std::array<XMFLOAT4, 4>& GetWallPlanes() { return m_xmf4MapBoundaryPlanes; }

private:
	std::shared_ptr<TerrainHeightMap>			m_pHeightMapImage = nullptr;
	std::vector<std::shared_ptr<TerrainMesh>>	m_pTerrainMeshes = {};

	std::array<std::shared_ptr<Texture>, 3>		m_pTerrainTextures;


	int m_nWidth = 0;
	int m_nLength = 0;
	XMFLOAT3 m_xmf3Scale;

	std::array<XMFLOAT4, 4>		m_xmf4MapBoundaryPlanes = {};
};

