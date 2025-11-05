#include "stdafx.h"
#include "Mesh.h"
#include "TerrainObject.h"

//////////
// Mesh //
//////////

Mesh::Mesh()
{
}

void Mesh::ReleaseUploadBuffers()
{
	if (m_pd3dPositionUploadBuffer) {
		m_pd3dPositionUploadBuffer.Reset();
	}

	if (m_nSubMeshes && m_pd3dSubSetIndexUploadBuffers.size() != 0) {
		for (int i = 0; i < m_nSubMeshes; ++i) {
			if (m_pd3dSubSetIndexUploadBuffers[i]) {
				m_pd3dSubSetIndexUploadBuffers[i].Reset();
			}
		}

		m_pd3dSubSetIndexUploadBuffers.clear();
	}
}

void Mesh::Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, int nSubSet, int nInstanceCount)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dPositionBufferView);
	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_d3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_nSubSetIndices[nSubSet], nInstanceCount, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, nInstanceCount, m_nOffset, 0);
	}
}

//////////////////
// StandardMesh //
//////////////////

void StandardMesh::LoadMeshFromFile(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, std::ifstream& inFile)
{
	std::string strRead;

	int nVertices;
	inFile.read((char*)(&m_nVertices), sizeof(int));
	m_strMeshName= ::ReadStringFromFile(inFile);

	while (true) {
		strRead = ::ReadStringFromFile(inFile);
		if (strRead == "<Bounds>:") {
			inFile.read((char*)&m_xmf3AABBCenter, sizeof(XMFLOAT3));
			inFile.read((char*)&m_xmf3AABBExtents, sizeof(XMFLOAT3));
			m_xmOBB.Center = m_xmf3AABBCenter;
			m_xmOBB.Extents = m_xmf3AABBExtents;
		}
		else if (strRead == "<Positions>:") {
			int nPositions;
			inFile.read((char*)&nPositions, sizeof(int));
			if (nPositions > 0) {
				m_nType |= VERTEX_TYPE_POSITION;
				m_xmf3Positions.resize(nPositions);
				inFile.read((char*)m_xmf3Positions.data(), sizeof(XMFLOAT3) * nPositions);

				m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_xmf3Positions.data(), m_xmf3Positions.size() * sizeof(XMFLOAT3),
					D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_pd3dPositionUploadBuffer.GetAddressOf());

				m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
				m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (strRead == "<Colors>:") {
			int nColors;
			inFile.read((char*)&nColors, sizeof(int));
			if (nColors > 0) {
				m_nType |= VERTEX_TYPE_COLOR;
				m_xmf4Colors.resize(nColors);
				inFile.read((char*)m_xmf4Colors.data(), sizeof(XMFLOAT4) * nColors);

				// ¹öÆÛ ¾È¸¸µë (¾È¾µ²¨ÀÓ)
			}
		}
		else if (strRead == "<TextureCoords0>:") {
			int nTexCoordss;
			inFile.read((char*)&nTexCoordss, sizeof(int));
			if (nTexCoordss > 0) {
				m_nType |= VERTEX_TYPE_TEXTURE_COORD0;
				m_xmf2TextureCoords0.resize(nTexCoordss);
				inFile.read((char*)m_xmf2TextureCoords0.data(), sizeof(XMFLOAT2) * nTexCoordss);

				m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_xmf2TextureCoords0.data(), m_xmf2TextureCoords0.size() * sizeof(XMFLOAT2),
					D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_pd3dTextureCoord0UploadBuffer.GetAddressOf());

				m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
				m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (strRead == "<TextureCoords1>:") {
			int nTexCoordss;
			inFile.read((char*)&nTexCoordss, sizeof(int));
			if (nTexCoordss > 1) {
				m_nType |= VERTEX_TYPE_TEXTURE_COORD1;
				m_xmf2TextureCoords1.resize(nTexCoordss);
				inFile.read((char*)m_xmf2TextureCoords1.data(), sizeof(XMFLOAT2) * nTexCoordss);

				m_pd3dTextureCoord1Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_xmf2TextureCoords1.data(), m_xmf2TextureCoords1.size() * sizeof(XMFLOAT2),
					D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_pd3dTextureCoord1UploadBuffer.GetAddressOf());

				m_d3dTextureCoord1BufferView.BufferLocation = m_pd3dTextureCoord1Buffer->GetGPUVirtualAddress();
				m_d3dTextureCoord1BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_d3dTextureCoord1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		else if (strRead == "<Normals>:") {
			int nNormals;
			inFile.read((char*)&nNormals, sizeof(int));
			if (nNormals > 0) {
				m_nType |= VERTEX_TYPE_NORMAL;
				m_xmf3Normals.resize(nNormals);
				inFile.read((char*)m_xmf3Normals.data(), sizeof(XMFLOAT3) * nNormals);

				m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_xmf3Normals.data(), m_xmf3Normals.size() * sizeof(XMFLOAT3),
					D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_pd3dNormalUploadBuffer.GetAddressOf());

				m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
				m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (strRead == "<Tangents>:") {
			int nTangents;
			inFile.read((char*)&nTangents, sizeof(int));
			if (nTangents > 0) {
				m_nType |= VERTEX_TYPE_TANGENT;
				m_xmf3Tangents.resize(nTangents);
				inFile.read((char*)m_xmf3Tangents.data(), sizeof(XMFLOAT3) * nTangents);

				m_pd3dTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_xmf3Tangents.data(), m_xmf3Tangents.size() * sizeof(XMFLOAT3),
					D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_pd3dTangentUploadBuffer.GetAddressOf());

				m_d3dTangentBufferView.BufferLocation = m_pd3dTangentBuffer->GetGPUVirtualAddress();
				m_d3dTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (strRead == "<BiTangents>:") {
			int nBiTangents;
			inFile.read((char*)&nBiTangents, sizeof(int));
			if (nBiTangents > 0) {
				m_xmf3BiTangents.resize(nBiTangents);
				inFile.read((char*)m_xmf3BiTangents.data(), sizeof(XMFLOAT3) * nBiTangents);

				m_pd3dBiTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_xmf3BiTangents.data(), m_xmf3BiTangents.size() * sizeof(XMFLOAT3),
					D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_pd3dBiTangentUploadBuffer.GetAddressOf());

				m_d3dBiTangentBufferView.BufferLocation = m_pd3dBiTangentBuffer->GetGPUVirtualAddress();
				m_d3dBiTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dBiTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}
		else if (strRead == "<SubMeshes>:") {
			int m_nSubMeshes;
			inFile.read((char*)&m_nSubMeshes, sizeof(int));
			if (m_nSubMeshes > 0) {
				m_nSubSetIndices.resize(m_nSubMeshes);
				m_IndicesBySubset.resize(m_nSubMeshes);

				m_pd3dSubSetIndexBuffers.resize(m_nSubMeshes);
				m_pd3dSubSetIndexUploadBuffers.resize(m_nSubMeshes);
				m_d3dSubSetIndexBufferViews.resize(m_nSubMeshes);

				for (int i = 0; i < m_nSubMeshes; ++i) {
					strRead = ::ReadStringFromFile(inFile);
					if (strRead == "<SubMesh>:") {
						int nIndex;
						inFile.read((char*)&nIndex, sizeof(int));
						inFile.read((char*)&m_nSubSetIndices[i], sizeof(int));
						if (m_nSubSetIndices[i] > 0) {
							m_IndicesBySubset[i].resize(m_nSubSetIndices[i]);
							inFile.read((char*)m_IndicesBySubset[i].data(), sizeof(UINT) * m_nSubSetIndices[i]);

							m_pd3dSubSetIndexBuffers[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_IndicesBySubset[i].data(), m_IndicesBySubset.size() * sizeof(UINT),
								D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_pd3dBiTangentUploadBuffer.GetAddressOf());


							m_d3dSubSetIndexBufferViews[i].BufferLocation = m_pd3dSubSetIndexBuffers[i]->GetGPUVirtualAddress();
							m_d3dSubSetIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
							m_d3dSubSetIndexBufferViews[i].SizeInBytes = sizeof(UINT) * m_nSubSetIndices[i];
						}
					}
				}
			}
		}
		else if ((strRead == "</Mesh>"))
		{
			break;
		}
	}
}

StandardMesh::StandardMesh()
{
}

StandardMesh::~StandardMesh()
{
}

void StandardMesh::ReleaseUploadBuffers()
{
	Mesh::ReleaseUploadBuffers();

	if (m_pd3dTextureCoord0UploadBuffer) {
		m_pd3dTextureCoord0UploadBuffer.Reset();
		m_pd3dTextureCoord0UploadBuffer = nullptr;
	}

	if (m_pd3dTextureCoord1UploadBuffer) {
		m_pd3dTextureCoord1UploadBuffer.Reset();
		m_pd3dTextureCoord1UploadBuffer = nullptr;
	}

	if (m_pd3dNormalUploadBuffer) {
		m_pd3dNormalUploadBuffer.Reset();
		m_pd3dNormalUploadBuffer = nullptr;
	}

	if (m_pd3dTangentUploadBuffer) {
		m_pd3dTangentUploadBuffer.Reset();
		m_pd3dTangentUploadBuffer = nullptr;
	}

	if (m_pd3dBiTangentUploadBuffer) {
		m_pd3dBiTangentUploadBuffer.Reset();
		m_pd3dBiTangentUploadBuffer = nullptr;
	}

}

void StandardMesh::Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, int nSubSet, int nInstanceCount)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	D3D12_VERTEX_BUFFER_VIEW d3dVertexBufferViews[] = {
		m_d3dPositionBufferView,
		m_d3dTextureCoord0BufferView,
		m_d3dNormalBufferView,
		m_d3dTangentBufferView,
		m_d3dBiTangentBufferView
	};

	pd3dCommandList->IASetVertexBuffers(m_nSlot, _countof(d3dVertexBufferViews), d3dVertexBufferViews);
	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_d3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_nSubSetIndices[nSubSet], nInstanceCount, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, nInstanceCount, m_nOffset, 0);
	}
}

TerrainMesh::TerrainMesh()
{
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
}

TerrainMesh::~TerrainMesh()
{
}

void TerrainMesh::Create(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, std::shared_ptr<TerrainHeightMap> pHeightMap)
{
	m_nVertices = nWidth * nLength;
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	int cxHeightMap = pHeightMap->GetHeightMapWidth();
	int czHeightMap = pHeightMap->GetHeightMapLength();

	m_xmf3Positions.resize(m_nVertices);
	m_xmf4Colors.resize(m_nVertices);
	m_xmf2TextureCoords0.resize(m_nVertices);
	m_xmf2TextureCoords1.resize(m_nVertices);

	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			fHeight = GetHeight(x, z, pHeightMap);
			m_xmf3Positions[i] = XMFLOAT3((x * m_xmf3Scale.x), fHeight, (z * m_xmf3Scale.z));
			m_xmf4Colors[i] = Vector4::Add(GetColor(x, z, pHeightMap), xmf4Color);
			m_xmf2TextureCoords0[i] = XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));
			m_xmf2TextureCoords1[i] = XMFLOAT2(float(x) / float(m_xmf3Scale.x * 0.5f), float(z) / float(m_xmf3Scale.z * 0.5f));
			if (fHeight < fMinHeight) fMinHeight = fHeight;
			if (fHeight > fMaxHeight) fMaxHeight = fHeight;
		}
	}

	m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_xmf3Positions.data(), sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_pd3dPositionUploadBuffer.GetAddressOf());
	m_d3dPositionBufferView.BufferLocation = m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_pd3dColorBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_xmf4Colors.data(), sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_pd3dColorUploadBuffer.GetAddressOf());
	m_d3dColorBufferView.BufferLocation = m_pd3dColorBuffer->GetGPUVirtualAddress();
	m_d3dColorBufferView.StrideInBytes = sizeof(XMFLOAT4);
	m_d3dColorBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;

	m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_xmf2TextureCoords0.data(), sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_pd3dTextureCoord0UploadBuffer.GetAddressOf());
	m_d3dTextureCoord0BufferView.BufferLocation = m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	m_pd3dTextureCoord1Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_xmf2TextureCoords1.data(), sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_pd3dTextureCoord1UploadBuffer.GetAddressOf());
	m_d3dTextureCoord1BufferView.BufferLocation = m_pd3dTextureCoord1Buffer->GetGPUVirtualAddress();
	m_d3dTextureCoord1BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_d3dTextureCoord1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	m_nSubMeshes = 1;
	m_nSubSetIndices.resize(m_nSubMeshes);
	m_IndicesBySubset.resize(m_nSubMeshes);

	m_pd3dSubSetIndexBuffers.resize(m_nSubMeshes);
	m_pd3dSubSetIndexUploadBuffers.resize(m_nSubMeshes);
	m_d3dSubSetIndexBufferViews.resize(m_nSubMeshes);

	m_nSubSetIndices[0] = ((nWidth * 2) * (nLength - 1)) + ((nLength - 1) - 1);
	m_IndicesBySubset[0].resize(m_nSubSetIndices[0]);

	for (int j = 0, z = 0; z < nLength - 1; z++)
	{
		if ((z % 2) == 0)
		{
			for (int x = 0; x < nWidth; x++)
			{
				if ((x == 0) && (z > 0)) m_IndicesBySubset[0][j++] = (UINT)(x + (z * nWidth));
				m_IndicesBySubset[0][j++] = (UINT)(x + (z * nWidth));
				m_IndicesBySubset[0][j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
		else
		{
			for (int x = nWidth - 1; x >= 0; x--)
			{
				if (x == (nWidth - 1)) m_IndicesBySubset[0][j++] = (UINT)(x + (z * nWidth));
				m_IndicesBySubset[0][j++] = (UINT)(x + (z * nWidth));
				m_IndicesBySubset[0][j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
	}

	m_pd3dSubSetIndexBuffers[0] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_IndicesBySubset[0].data(), sizeof(UINT) * m_nSubSetIndices[0], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, m_pd3dSubSetIndexUploadBuffers[0].GetAddressOf());

	m_d3dSubSetIndexBufferViews[0].BufferLocation = m_pd3dSubSetIndexBuffers[0]->GetGPUVirtualAddress();
	m_d3dSubSetIndexBufferViews[0].Format = DXGI_FORMAT_R32_UINT;
	m_d3dSubSetIndexBufferViews[0].SizeInBytes = sizeof(UINT) * m_nSubSetIndices[0];
}

void TerrainMesh::ReleaseUploadBuffers()
{
	Mesh::ReleaseUploadBuffers();

	if (m_pd3dTextureCoord0UploadBuffer) {
		m_pd3dTextureCoord0UploadBuffer.Reset();
		m_pd3dTextureCoord0UploadBuffer = nullptr;
	}

	if (m_pd3dTextureCoord1UploadBuffer) {
		m_pd3dTextureCoord1UploadBuffer.Reset();
		m_pd3dTextureCoord1UploadBuffer = nullptr;
	}

	if (m_pd3dColorBuffer) {
		m_pd3dColorBuffer.Reset();
		m_pd3dColorBuffer = nullptr;
	}

}

void TerrainMesh::Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, int nSubSet, int nInstanceCount)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	D3D12_VERTEX_BUFFER_VIEW d3dVertexBufferViews[] = {
		m_d3dPositionBufferView,
		m_d3dColorBufferView,
		m_d3dTextureCoord0BufferView,
		m_d3dTextureCoord1BufferView,
	};

	pd3dCommandList->IASetVertexBuffers(m_nSlot, _countof(d3dVertexBufferViews), d3dVertexBufferViews);
	if ((m_nSubMeshes > 0) && (nSubSet < m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_d3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_nSubSetIndices[nSubSet], nInstanceCount, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, nInstanceCount, m_nOffset, 0);
	}
}

float TerrainMesh::GetHeight(int x, int z, std::shared_ptr<TerrainHeightMap> pHeightMap)
{
	std::vector<BYTE>& pHeightMapPixels = pHeightMap->GetHeightMapPixels();
	XMFLOAT3 xmf3Scale = pHeightMap->GetScale();
	int nWidth = pHeightMap->GetHeightMapWidth();
	float fHeight = pHeightMapPixels[x + (z * nWidth)] * xmf3Scale.y;
	return fHeight;
}

XMFLOAT4 TerrainMesh::GetColor(int x, int z, std::shared_ptr<TerrainHeightMap> pHeightMap)
{
	XMFLOAT3 xmf3LightDirection = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	xmf3LightDirection = Vector3::Normalize(xmf3LightDirection);
	XMFLOAT3 xmf3Scale = pHeightMap->GetScale();
	XMFLOAT4 xmf4IncidentLightColor(0.9f, 0.8f, 0.4f, 1.0f);
	float fScale = Vector3::DotProduct(pHeightMap->GetHeightMapNormal(x, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMap->GetHeightMapNormal(x + 1, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMap->GetHeightMapNormal(x + 1, z + 1), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMap->GetHeightMapNormal(x, z + 1), xmf3LightDirection);
	fScale = (fScale / 4.0f) + 0.05f;
	if (fScale > 1.0f) fScale = 1.0f;
	if (fScale < 0.25f) fScale = 0.25f;
	XMFLOAT4 xmf4Color;
	XMStoreFloat4(&xmf4Color, fScale * XMLoadFloat4(&xmf4IncidentLightColor));
	return xmf4Color;
}
