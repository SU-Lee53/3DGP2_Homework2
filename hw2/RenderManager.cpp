#include "stdafx.h"
#include "RenderManager.h"
#include "GameObject.h"
#include "Material.h"
#include "StructuredBuffer.h"
#include "UIManager.h"

ComPtr<ID3D12RootSignature> RenderManager::g_pd3dRootSignature = nullptr;

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
	heapDesc.NumDescriptors = ASSUMED_REQUIRED_DESCRIPTOR_COUNT;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapDesc.NodeMask = 0;

	m_pd3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_pd3dDescriptorHeap.GetAddressOf()));

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

void RenderManager::Render(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRootSignature(g_pd3dRootSignature.Get());
	RENDER->SetDescriptorHeapToPipeline(pd3dCommandList);
	CUR_SCENE->UpdateShaderVariable(pd3dCommandList);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dCPUHandle = m_pd3dDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE d3dGPUHandle = m_pd3dDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	DescriptorHandle descHandle{ d3dCPUHandle, d3dGPUHandle };


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

	RenderObjects(pd3dCommandList, descHandle);

}

void RenderManager::SetDescriptorHeapToPipeline(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList) const
{
	pd3dCommandList->SetDescriptorHeaps(1, m_pd3dDescriptorHeap.GetAddressOf());
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

	// Instance 행렬 정보를 한번에 GPU에 바인딩
#ifdef INSTANCING_USING_DESCRIPTOR_TABLE
	m_pd3dDevice->CopyDescriptorsSimple(1, refDescHandle.cpuHandle,
		m_InstanceDataSBuffer.GetCPUDescriptorHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	refDescHandle.cpuHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

	pd3dCommandList->SetGraphicsRootDescriptorTable(2, refDescHandle.gpuHandle);
	refDescHandle.gpuHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

#else
	m_InstanceDataSBuffer.SetBufferToPipeline(pd3dCommandList, 2);

#endif // INSTANCING_USING_DESCRIPTOR_TABLE

	int nInstanceBase = 0;
	int nInstanceCount = 0;
	for (auto&& [instanceKey, instanceData] : m_InstanceDatas) {
		nInstanceCount = instanceData.size();
		pd3dCommandList->SetGraphicsRoot32BitConstant(3, nInstanceBase, 0);

		for (int i = 0; i < instanceKey.pMaterials.size(); ++i) {
			instanceKey.pMaterials[i]->UpdateShaderVariable(pd3dCommandList);
			m_pd3dDevice->CopyDescriptorsSimple(1, refDescHandle.cpuHandle,
				instanceKey.pMaterials[i]->GetCBuffer().GetCPUDescriptorHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			refDescHandle.cpuHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

			instanceKey.pMaterials[i]->OnPrepareRender(pd3dCommandList);

			pd3dCommandList->SetGraphicsRootDescriptorTable(1, refDescHandle.gpuHandle);
			refDescHandle.gpuHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

			instanceKey.pMesh->Render(pd3dCommandList, i, nInstanceCount);
		}

		nInstanceBase += nInstanceCount;
	}

}

void RenderManager::CreateGlobalRootSignature(ComPtr<ID3D12Device> pd3dDevice)
{
	CD3DX12_DESCRIPTOR_RANGE d3dDescriptorRanges[6];
	d3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 0, 0, 0);	// c0, c1 : Camera, Lights
	d3dDescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 0);	// t0 : Skybox
	d3dDescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 1, 0, 0);	// t1, t2, t3 : 각각 Terrain, Detail Terrain, 물

	d3dDescriptorRanges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, 0, 0);	// c2 : Material, StructuredBuffer 에서 World 행렬의 위치(Base)
	d3dDescriptorRanges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 4, 0, 0);	// t4 ~ t10 : 각각 albedo, specular, normal, metallic, emission, detail albedo, detail normal

	d3dDescriptorRanges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 11, 0, 0);	// t11 : World 행렬들을 전부 담을 StructuredBuffer

	CD3DX12_ROOT_PARAMETER d3dRootParameters[4];
	{
		// Per Scene
		d3dRootParameters[0].InitAsDescriptorTable(3, &d3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_ALL);
		
		// Material
		d3dRootParameters[1].InitAsDescriptorTable(2, &d3dDescriptorRanges[3], D3D12_SHADER_VISIBILITY_ALL);
		
		// Instance data
		d3dRootParameters[2].InitAsDescriptorTable(1, &d3dDescriptorRanges[5], D3D12_SHADER_VISIBILITY_ALL);

		// 디버그용 OBB 그리기
		d3dRootParameters[3].InitAsConstantBufferView(3, 0, D3D12_SHADER_VISIBILITY_ALL);
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
