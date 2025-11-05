#include "stdafx.h"
#include "TerrainObject.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TerrainHeightMap

TerrainHeightMap::TerrainHeightMap(std::string_view svFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	std::string strFilePath{ svFileName };
	std::ifstream in{ strFilePath, std::ios::binary };
	if (!in)
		__debugbreak();

	std::vector<BYTE> pixelReads;

	while (in) {
		BYTE b;
		in.read((char*)&b, sizeof(BYTE));
		pixelReads.push_back(b);
	}

	m_HeightMapPixels.resize(pixelReads.size());

	for (int y = 0; y < m_nLength; ++y) {
		for (int x = 0; x < m_nWidth; ++x) {
			m_HeightMapPixels[x + ((m_nLength - 1 - y) * m_nWidth)] = pixelReads[x + (y * m_nWidth)];
		}
	}
}

TerrainHeightMap::~TerrainHeightMap()
{
}

float TerrainHeightMap::GetHeight(float fx, float fz)
{
	if ((fx < 0.f) || (fz < 0.f) || (fx >= m_nWidth) || (fz >= m_nLength)) {
		return 0.0f;
	}

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)m_HeightMapPixels[x + (z * m_nWidth)];
	float fBottomRight = (float)m_HeightMapPixels[(x + 1) + (z * m_nWidth)];
	float fTopLeft = (float)m_HeightMapPixels[x + ((z + 1) * m_nWidth)];
	float fTopRight = (float)m_HeightMapPixels[(x + 1) + ((z + 1) * m_nWidth)];

	bool bRightToLeft = ((z % 2) != 0);
	if (bRightToLeft) {
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else {
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}

	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return fHeight;
}

XMFLOAT3 TerrainHeightMap::GetHeightMapNormal(int x, int z)
{
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength)) {
		return XMFLOAT3(0.f, 1.f, 0.f);
	}

	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;

	float y1 = (float)m_HeightMapPixels[nHeightMapIndex] * m_xmf3Scale.y;
	float y2 = (float)m_HeightMapPixels[nHeightMapIndex + xHeightMapAdd] * m_xmf3Scale.y;
	float y3 = (float)m_HeightMapPixels[nHeightMapIndex + zHeightMapAdd] * m_xmf3Scale.y;

	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.f, y3 - y1, m_xmf3Scale.z);
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(m_xmf3Scale.x, y2 - y1, 0.f);

	XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true);

	return xmf3Normal;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TerrainObject

TerrainObject::TerrainObject()
{
}

TerrainObject::~TerrainObject()
{
}

void TerrainObject::Initialize(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, const std::string& strFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;
	m_pHeightMapImage = make_shared<TerrainHeightMap>(strFileName, nWidth, nLength, xmf3Scale);

	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;

	m_pTerrainMeshes.resize(cxBlocks * czBlocks);

	std::shared_ptr<TerrainMesh> pHeightMapGridMesh;
	for (int z = 0, zStart = 0; z < czBlocks; ++z) {
		for (int x = 0, xStart = 0; x < cxBlocks; ++x) {
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);

			pHeightMapGridMesh = make_shared<TerrainMesh>();
			pHeightMapGridMesh->Create(pd3dDevice, pd3dCommandList,
				xStart, zStart, nBlockWidth, nBlockLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
			m_pTerrainMeshes[x + (z * cxBlocks)] = pHeightMapGridMesh;
		}
	}

	XMStoreFloat4(&m_xmf4MapBoundaryPlanes[0], XMPlaneFromPointNormal(XMVectorSet(0.f, 0.f, 0.f, 1.f), XMVectorSet(1.f, 0.f, 0.f, 0.f)));
	XMStoreFloat4(&m_xmf4MapBoundaryPlanes[1], XMPlaneFromPointNormal(XMVectorSet((GetWidth() * 0.5f), 0.f, 0.f, 1.f), XMVectorSet(-1.f, 0.f, 0.f, 0.f)));

	XMStoreFloat4(&m_xmf4MapBoundaryPlanes[2], XMPlaneFromPointNormal(XMVectorSet(0.f, 0.f, 0.f, 1.f), XMVectorSet(0.f, 0.f, 1.f, 0.f)));
	XMStoreFloat4(&m_xmf4MapBoundaryPlanes[3], XMPlaneFromPointNormal(XMVectorSet(0.f, 0.f, (GetLength() * 0.5f), 1.f), XMVectorSet(0.f, 0.f, -1.f, 0.f)));

	TEXTURE->LoadTexture(pd3dCommandList, "TerrainBase", L"Terrain/Base_Texture.dds", RESOURCE_TYPE_TEXTURE2D);
	TEXTURE->LoadTexture(pd3dCommandList, "TerrainDetailed", L"Terrain/Detail_Texture_7.dds", RESOURCE_TYPE_TEXTURE2D);
	TEXTURE->LoadTexture(pd3dCommandList, "TerrainWater", L"Terrain/Water_Base_Texture_0.dds", RESOURCE_TYPE_TEXTURE2D);

	// 11.05
	// TODO : 여기부터

	//m_xmOBB.Extents = XMFLOAT3{ (GetLength() * 0.5f), 500.f, (GetLength() * 0.5f) };
	//XMStoreFloat4(&m_xmOBB.Orientation, XMQuaternionRotationRollPitchYaw(0.f, 0.f, 0.f));
	//m_xmOBB.Center = XMFLOAT3{ GetWidth() * 0.5f, GetHeight(GetWidth() * 0.5, GetLength() * 0.5f), GetLength() * 0.5f };





}

void TerrainObject::Render(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, DescriptorHandle& refDescHandle)
{
}
