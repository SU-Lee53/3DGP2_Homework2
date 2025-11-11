#include "stdafx.h"
#include "RenderManager.h"
#include "Material.h"
#include "StructuredBuffer.h"
#include "UIManager.h"
#include "TerrainObject.h"
#include "BuildingObject.h"	// MirrorObject

ComPtr<ID3D12RootSignature> RenderManager::g_pd3dRootSignature = nullptr;
bool RenderManager::g_bRenderOBBForDebug = true;

RenderManager::RenderManager(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	m_pd3dDevice = pd3dDevice;

#ifdef INSTANCING_USING_DESCRIPTOR_TABLE
	m_InstanceDataSBuffer.Create(pd3dDevice, pd3dCommandList, ASSUMED_REQUIRED_STRUCTURED_BUFFER_SIZE, sizeof(INSTANCE_DATA), true);

#else
	m_InstanceDataSBuffer.Create(pd3dDevice, pd3dCommandList, ASSUMED_INSTANCE_COUNT, sizeof(INSTANCE_DATA), false);

#endif
	 
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.NumDescriptors = 10000;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;

	m_pd3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_pd3dDescriptorHeap.GetAddressOf()));

	m_DescriptorHandle.cpuHandle = m_pd3dDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_DescriptorHandle.gpuHandle = m_pd3dDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	CreateGlobalRootSignature(pd3dDevice);
}

void RenderManager::Add(std::shared_ptr<GameObject> pGameObject)
{
	INSTANCE_KEY key{ pGameObject->GetMesh(), pGameObject->GetMaterials(), key.pMesh->GetSubSetCount() };
	XMFLOAT4X4 xmf4x4InstanceData;
	XMStoreFloat4x4(&xmf4x4InstanceData, XMMatrixTranspose(XMLoadFloat4x4(&pGameObject->GetWorldMatrix())));

	auto it = m_InstanceIndexMap.find(key);
	if (it == m_InstanceIndexMap.end()) {
		m_InstanceIndexMap[key] = m_nInstanceIndex++;
		m_InstanceDatas.push_back({ key, {} });
		m_nDrawCalls += key.pMaterials.size();

		m_InstanceDatas[m_InstanceIndexMap[key]].second.emplace_back(xmf4x4InstanceData);
	}
	else {
		m_InstanceDatas[it->second].second.emplace_back(xmf4x4InstanceData);
	}

}

void RenderManager::AddMirror(std::shared_ptr<MirrorObject> pMirrorObject)
{
	// TODO : Make it Happen
}

void RenderManager::AddTransparent(std::shared_ptr<MirrorObject> pMirrorObject)
{
	// TODO : Make it Happen
}

void RenderManager::Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRootSignature(g_pd3dRootSignature.Get());
	pd3dCommandList->SetDescriptorHeaps(1, m_pd3dDescriptorHeap.GetAddressOf());
	CUR_SCENE->UpdateShaderVariable(pd3dCommandList);

	DescriptorHandle descHandle = m_DescriptorHandle;

	// Per Scene Descriptor 에 복사
	auto pCamera = CUR_SCENE->GetPlayer()->GetCamera();
	pCamera->UpdateShaderVariables(pd3dCommandList);
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	m_pd3dDevice->CopyDescriptorsSimple(1, descHandle.cpuHandle,
		pCamera->GetCBuffer().GetCPUDescriptorHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descHandle.cpuHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

	m_pd3dDevice->CopyDescriptorsSimple(1, descHandle.cpuHandle,
		CUR_SCENE->GetLightCBuffer().GetCPUDescriptorHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descHandle.cpuHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

	pd3dCommandList->SetGraphicsRootDescriptorTable(0, descHandle.gpuHandle);
	descHandle.gpuHandle.ptr += 2 * GameFramework::g_uiDescriptorHandleIncrementSize;

	// Skybox 추가 필요
	// 일단 임시로 1칸 증가시킴
	descHandle.cpuHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;
	descHandle.gpuHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

	RenderObjects(pd3dCommandList, descHandle);


	// TODO 추가
	// RenderTerrain
	if (m_pTerrain) {
		RenderTerrain(pd3dCommandList, descHandle);
	}
	// RenderSkybox
}

void RenderManager::SetDescriptorHeapToPipeline(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList) const
{
	pd3dCommandList->SetDescriptorHeaps(1, m_pd3dDescriptorHeap.GetAddressOf());
}

void RenderManager::RenderObjectsInMirrorWorld(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, DescriptorHandle& refDescHandle, const XMFLOAT4& xmf4MirrorPlane)
{
	// TODO : Make it Happen
}

void RenderManager::RenderObjects(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, DescriptorHandle& refDescHandle)
{
	UINT uiSBufferOffset = 0;
	for (auto&& [instanceKey, instanceData] : m_InstanceDatas) {
		m_InstanceDataSBuffer.UpdateData(instanceData, uiSBufferOffset);
		uiSBufferOffset += instanceData.size();
	}
#ifdef WITH_UPLOAD_BUFFER
	m_InstanceDataSBuffer.UpdateResources(m_pd3dDevice, pd3dCommandList);

#endif

	m_pd3dDevice->CopyDescriptorsSimple(1, refDescHandle.cpuHandle,
		m_InstanceDataSBuffer.GetCPUDescriptorHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	refDescHandle.cpuHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

	pd3dCommandList->SetGraphicsRootDescriptorTable(3, refDescHandle.gpuHandle);
	refDescHandle.gpuHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

	int nInstanceBase = 0;
	int nInstanceCount = 0;
	for (auto& [instanceKey, instanceData] : m_InstanceDatas) {
		nInstanceCount = instanceData.size();

		for (int i = 0; i < instanceKey.pMaterials.size(); ++i) {
			// Color + nType
			instanceKey.pMaterials[i]->UpdateShaderVariable(pd3dCommandList, nInstanceBase);
			m_pd3dDevice->CopyDescriptorsSimple(1, refDescHandle.cpuHandle,
				instanceKey.pMaterials[i]->GetCBuffer().GetCPUDescriptorHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			refDescHandle.cpuHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

			// Textures
			instanceKey.pMaterials[i]->CopyTextureDescriptors(m_pd3dDevice, refDescHandle);

			// Descriptor Set
			pd3dCommandList->SetGraphicsRootDescriptorTable(4, refDescHandle.gpuHandle);
			refDescHandle.gpuHandle.ptr += (1 + Material::g_nTexturesPerMaterial) * GameFramework::g_uiDescriptorHandleIncrementSize;
			// 1 (CB_MATERIAL_DATA) + Texture 7개 

			instanceKey.pMaterials[i]->OnPrepareRender(pd3dCommandList);
			instanceKey.pMesh->Render(pd3dCommandList, i, nInstanceCount);
		}

		nInstanceBase += nInstanceCount;
	}

}

void RenderManager::RenderTerrain(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, DescriptorHandle& refDescHandle)
{
	m_pTerrain->Render(m_pd3dDevice, pd3dCommandList, refDescHandle);
}

void RenderManager::CreateGlobalRootSignature(ComPtr<ID3D12Device> pd3dDevice)
{
	CD3DX12_DESCRIPTOR_RANGE d3dDescriptorRanges[8];
	d3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0, 0, 0);	// b0, b1 : Camera, Lights
	d3dDescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 0);	// t0 : Skybox

	d3dDescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, 0, 0);	// b2 : Terrain world matrix
	d3dDescriptorRanges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3, 0, 1);	// b3 : Billboard datas
	d3dDescriptorRanges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 1, 0, 2);	// t1 ~ t3 : terrain billboards
	
	d3dDescriptorRanges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0, 0);	// t4 : World 행렬들을 전부 담을 StructuredBuffer

	d3dDescriptorRanges[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 4, 0, 0);	// b4 : Material, StructuredBuffer 에서 World 행렬의 위치(Base)
	d3dDescriptorRanges[7].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 5, 0, 1);	// t5 ~ t11 : 각각 albedo, specular, normal, metallic, emission, detail albedo, detail normal


	CD3DX12_ROOT_PARAMETER d3dRootParameters[6];
	{
		// Per Scene
		d3dRootParameters[0].InitAsDescriptorTable(1, &d3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_ALL);	// b0, b1
		d3dRootParameters[1].InitAsDescriptorTable(1, &d3dDescriptorRanges[1], D3D12_SHADER_VISIBILITY_ALL);	// t0

		// Terrain
		d3dRootParameters[2].InitAsDescriptorTable(3, &d3dDescriptorRanges[2], D3D12_SHADER_VISIBILITY_ALL);	// b2, b3 / t1 ~ t3

		// Instance data
		d3dRootParameters[3].InitAsDescriptorTable(1, &d3dDescriptorRanges[5], D3D12_SHADER_VISIBILITY_ALL);	// t8

		// Material
		d3dRootParameters[4].InitAsDescriptorTable(2, &d3dDescriptorRanges[6], D3D12_SHADER_VISIBILITY_ALL);	// b3 / t9 ~ t15

		// 디버그용 OBB 그리기
		d3dRootParameters[5].InitAsConstants(16, 5, 0, D3D12_SHADER_VISIBILITY_ALL);	// Center + Extent + Orientation + Color
	}

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

	D3D12_STATIC_SAMPLER_DESC d3dSamplerDescs[2];
	d3dSamplerDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	d3dSamplerDescs[0].MipLODBias = 0;
	d3dSamplerDescs[0].MaxAnisotropy = 1;
	d3dSamplerDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDescs[0].MinLOD = 0;
	d3dSamplerDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDescs[0].ShaderRegister = 0;
	d3dSamplerDescs[0].RegisterSpace = 0;
	d3dSamplerDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	d3dSamplerDescs[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	d3dSamplerDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	d3dSamplerDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	d3dSamplerDescs[1].MipLODBias = 0;
	d3dSamplerDescs[1].MaxAnisotropy = 1;
	d3dSamplerDescs[1].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	d3dSamplerDescs[1].MinLOD = 0;
	d3dSamplerDescs[1].MaxLOD = D3D12_FLOAT32_MAX;
	d3dSamplerDescs[1].ShaderRegister = 1;
	d3dSamplerDescs[1].RegisterSpace = 0;
	d3dSamplerDescs[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc{};
	{
		d3dRootSignatureDesc.NumParameters = _countof(d3dRootParameters);
		d3dRootSignatureDesc.pParameters = d3dRootParameters;
		d3dRootSignatureDesc.NumStaticSamplers = _countof(d3dSamplerDescs);
		d3dRootSignatureDesc.pStaticSamplers = d3dSamplerDescs;
		d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;
	}

	ComPtr<ID3DBlob> pd3dSignatureBlob = nullptr;
	ComPtr<ID3DBlob> pd3dErrorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, pd3dSignatureBlob.GetAddressOf(), pd3dErrorBlob.GetAddressOf());
	if (FAILED(hr)) {
		char* pErrorString = (char*)pd3dErrorBlob->GetBufferPointer();
		HWND hWnd = ::GetActiveWindow();
		MessageBoxA(hWnd, pErrorString, NULL, 0);
		OutputDebugStringA(pErrorString);
		__debugbreak();
	}

	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), IID_PPV_ARGS(g_pd3dRootSignature.GetAddressOf()));
}

void RenderManager::Clear()
{
	m_InstanceIndexMap.clear();
	m_InstanceDatas.clear();
	m_nInstanceIndex = 0;
	m_nDrawCalls = 0;

}

size_t RenderManager::GetMeshCount() const
{
#ifdef INSTANCES_IN_HASHMAP
	return m_InstanceMap.size();

#else
	return m_InstanceIndexMap.size();

#endif

}
