#include "stdafx.h"
#include "TerrainObject.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RawFormatImage

RawFormatImage::RawFormatImage(const std::string& strFileName, int nWidth, int nLength, bool bFlipY)
{
	m_nWidth = nWidth;
	m_nLength = nLength;

	std::ifstream in{ strFileName, std::ios::binary };
	if (!in)
		__debugbreak();

	std::vector<BYTE> pixelReads;

	while (in) {
		BYTE b;
		in.read((char*)&b, sizeof(BYTE));
		pixelReads.push_back(b);
	}


	if (bFlipY) {
		m_pRawImagePixels.resize(pixelReads.size());
		for (int z = 0; z < m_nLength; ++z) {
			for (int x = 0; x < m_nWidth; ++x) {
				m_pRawImagePixels[x + ((m_nLength - 1 - z) * m_nWidth)] = pixelReads[x + (z * m_nWidth)];
			}
		}
	}
	else {
		m_pRawImagePixels = pixelReads;
	}
}

RawFormatImage::~RawFormatImage(void)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HeightMapRawImage

HeightMapRawImage::HeightMapRawImage(const std::string& strFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
	: RawFormatImage(strFileName, nWidth, nLength, true)
{
	m_xmf3Scale = xmf3Scale;
}

HeightMapRawImage::~HeightMapRawImage()
{
}

float HeightMapRawImage::GetHeight(float fx, float fz, bool bReverseQuad)
{
	fx = fx / m_xmf3Scale.x;
	fz = fz / m_xmf3Scale.z;
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)m_pRawImagePixels[x + (z * m_nWidth)];
	float fBottomRight = (float)m_pRawImagePixels[(x + 1) + (z * m_nWidth)];
	float fTopLeft = (float)m_pRawImagePixels[x + ((z + 1) * m_nWidth)];
	float fTopRight = (float)m_pRawImagePixels[(x + 1) + ((z + 1) * m_nWidth)];
	if (bReverseQuad)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight);
}

XMFLOAT3 HeightMapRawImage::GetHeightMapNormal(int x, int z)
{
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength)) {
		return XMFLOAT3(0.f, 1.f, 0.f);
	}

	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;

	float y1 = (float)m_pRawImagePixels[nHeightMapIndex] * m_xmf3Scale.y;
	float y2 = (float)m_pRawImagePixels[nHeightMapIndex + xHeightMapAdd] * m_xmf3Scale.y;
	float y3 = (float)m_pRawImagePixels[nHeightMapIndex + zHeightMapAdd] * m_xmf3Scale.y;

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
	m_pHeightMapImage = make_shared<HeightMapRawImage>(strFileName, nWidth, nLength, xmf3Scale);

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

	m_xmOBB.Extents = XMFLOAT3{ (GetLength() * 0.5f), 500.f, (GetLength() * 0.5f) };
	XMStoreFloat4(&m_xmOBB.Orientation, XMQuaternionRotationRollPitchYaw(0.f, 0.f, 0.f));
	m_xmOBB.Center = XMFLOAT3{ GetWidth() * 0.5f, GetHeight(GetWidth() * 0.5, GetLength() * 0.5f), GetLength() * 0.5f };

	std::shared_ptr<Texture> pBaseTexture = TEXTURE->LoadTexture(pd3dCommandList, "TerrainBase", L"Terrain/Base_Texture.dds", RESOURCE_TYPE_TEXTURE2D);
	std::shared_ptr<Texture> pDetailedTexture = TEXTURE->LoadTexture(pd3dCommandList, "TerrainDetailed", L"Terrain/Detail_Texture_7.dds", RESOURCE_TYPE_TEXTURE2D);

	std::shared_ptr<Material> pTerrainMaterial = std::make_shared<Material>(pd3dDevice, pd3dCommandList);
	pTerrainMaterial->SetTexture(TEXTURE_INDEX_ALBEDO_MAP, pBaseTexture);
	pTerrainMaterial->SetTexture(TEXTURE_INDEX_DETAIL_ALBEDO_MAP, pDetailedTexture);

	std::shared_ptr<TerrainShader> pTerrainShader = std::make_shared<TerrainShader>();
	pTerrainShader->Create(pd3dDevice);
	pTerrainMaterial->SetShader(pTerrainShader);

	m_pMaterials.push_back(pTerrainMaterial);

	m_TerrainCBuffer.Create(pd3dDevice, pd3dCommandList, ConstantBufferSize<CB_TERRAIN_DATA>::value, true);

	// 11.06
	// TODO : 여기부터
	// 자식 오브젝트 하나 생성
	// 물 그릴 Grid Mesh 하나와 물 텍스쳐를 준비
	CreateChildWaterGridObject(pd3dDevice, pd3dCommandList, nWidth, nLength, nBlockWidth, nBlockLength, xmf3Scale, xmf4Color);

	m_pChildTerrain = static_pointer_cast<TerrainObject>(m_pChildren[0]);

	UpdateTransform();
}

void TerrainObject::CreateChildWaterGridObject(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color)
{
	std::shared_ptr<TerrainObject> pWaterGridObject = std::make_shared<TerrainObject>();

	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;

	pWaterGridObject->m_pTerrainMeshes.resize(cxBlocks * czBlocks);

	std::shared_ptr<TerrainMesh> pHeightMapGridMesh;
	for (int z = 0, zStart = 0; z < czBlocks; ++z) {
		for (int x = 0, xStart = 0; x < cxBlocks; ++x) {
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);

			pHeightMapGridMesh = make_shared<TerrainMesh>();
			pHeightMapGridMesh->Create(pd3dDevice, pd3dCommandList,
				xStart, zStart, nBlockWidth, nBlockLength, m_xmf3Scale, xmf4Color, nullptr);
			pWaterGridObject->m_pTerrainMeshes[x + (z * cxBlocks)] = pHeightMapGridMesh;
		}
	}

	std::shared_ptr<Texture> pBaseTexture = TEXTURE->LoadTexture(pd3dCommandList, "WaterBase", L"Terrain/Water_Base_Texture_0.dds", RESOURCE_TYPE_TEXTURE2D);
	std::shared_ptr<Texture> pDetailedTexture = TEXTURE->LoadTexture(pd3dCommandList, "WaterDetailed", L"Terrain/Water_Detail_Texture_0.dds", RESOURCE_TYPE_TEXTURE2D);

	std::shared_ptr<Material> pWaterMaterial = std::make_shared<Material>(pd3dDevice, pd3dCommandList);
	pWaterMaterial->SetTexture(TEXTURE_INDEX_ALBEDO_MAP, pBaseTexture);
	pWaterMaterial->SetTexture(TEXTURE_INDEX_DETAIL_ALBEDO_MAP, pDetailedTexture);

	pWaterMaterial->SetShader(m_pMaterials[0]->GetShader());

	pWaterGridObject->m_pMaterials.push_back(pWaterMaterial);

	pWaterGridObject->SetName("WaterGridFrame");
	pWaterGridObject->m_TerrainCBuffer.Create(pd3dDevice, pd3dCommandList, ConstantBufferSize<CB_TERRAIN_DATA>::value, true);

	XMStoreFloat4x4(&pWaterGridObject->m_xmf4x4Transform, XMMatrixTranslation(0.f, 250.f, 0.f));

	SetChild(pWaterGridObject);
}

void TerrainObject::CreateBillboards(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, const std::string& strFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color)
{
	// TODO : Make it happen

	std::shared_ptr<HeightMapRawImage> pObjectMapRaw = make_shared<HeightMapRawImage>(strFileName, nWidth, nLength, xmf3Scale);

}

void TerrainObject::Update(float fTimeElapsed)
{
	m_pChildTerrain->m_xmf2UVTranslation.x += 0.1f * fTimeElapsed;
	m_pChildTerrain->m_xmf2UVTranslation.y += 0.1f * fTimeElapsed;
}

void TerrainObject::Render(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, DescriptorHandle& refDescHandle)
{
	if ((m_pMaterials.size() == 1) && (m_pMaterials[0]))
	{
		CB_TERRAIN_DATA terrainData;
		{
			XMStoreFloat4x4(&terrainData.xmf4x4TerrainWorld, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
			terrainData.xmf2UVTranslation = m_xmf2UVTranslation;
		}
		m_TerrainCBuffer.UpdateData(&terrainData);

		pd3dDevice->CopyDescriptorsSimple(1, refDescHandle.cpuHandle,
			m_TerrainCBuffer.GetCPUDescriptorHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		refDescHandle.cpuHandle.ptr += 1 * GameFramework::g_uiDescriptorHandleIncrementSize;

		pd3dCommandList->SetGraphicsRootDescriptorTable(1, refDescHandle.gpuHandle);
		refDescHandle.gpuHandle.ptr += 1 * GameFramework::g_uiDescriptorHandleIncrementSize;


		if (m_pMaterials[0]->GetShader()) m_pMaterials[0]->GetShader()->OnPrepareRender(pd3dCommandList);

		// Color + nType
		m_pMaterials[0]->UpdateShaderVariable(pd3dCommandList, 1);
		pd3dDevice->CopyDescriptorsSimple(1, refDescHandle.cpuHandle,
			m_pMaterials[0]->GetCBuffer().GetCPUDescriptorHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		refDescHandle.cpuHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

		// Textures
		m_pMaterials[0]->CopyTextureDescriptors(pd3dDevice, refDescHandle);

		// Descriptor Set
		pd3dCommandList->SetGraphicsRootDescriptorTable(4, refDescHandle.gpuHandle);
		refDescHandle.gpuHandle.ptr += (1 + Material::g_nTexturesPerMaterial) * GameFramework::g_uiDescriptorHandleIncrementSize;
		// 1 (CB_MATERIAL_DATA) + Texture 7개 
	}

	if (m_pTerrainMeshes.size() >= 1)
	{
		for (int i = 0; i < m_pTerrainMeshes.size(); i++)
		{
			if (m_pTerrainMeshes[i]) m_pTerrainMeshes[i]->Render(pd3dCommandList, 0);
		}
	}

	for (auto& pChild : m_pChildren) {
		static_pointer_cast<TerrainObject>(pChild)->Render(pd3dDevice, pd3dCommandList, refDescHandle);
	}

}
