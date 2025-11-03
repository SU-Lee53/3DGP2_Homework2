#include "stdafx.h"
#include "Scene.h"

Scene::Scene(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
}

void Scene::BuildDefaultLightsAndMaterials()
{
}

void Scene::BuildObjects(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	CreateRootSignature(pd3dDevice);
	Material::PrepareShaders(pd3dDevice, m_pd3dRootSignature);
	BuildDefaultLightsAndMaterials();

	std::shared_ptr<AirplanePlayer> pAirplanePlayer = std::make_shared<AirplanePlayer>(pd3dDevice, pd3dCommandList, m_pd3dRootSignature);
	std::shared_ptr<ThirdPersonCamera> pCamera = std::make_shared<ThirdPersonCamera>();

	pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pCamera->SetTimeLag(0.25f);
	pCamera->SetOffset(XMFLOAT3(0.0f, 105.0f, -140.0f));
	pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
	pCamera->SetViewport(0, 0, GameFramework::g_nClientWidth, GameFramework::g_nClientHeight, 0.0f, 1.0f);
	pCamera->SetScissorRect(0, 0, GameFramework::g_nClientWidth, GameFramework::g_nClientHeight);
	pCamera->SetPlayer(pAirplanePlayer);
	pAirplanePlayer->SetCamera(pCamera);

	m_pPlayer = pAirplanePlayer;
	m_pPlayer->Initialize();

	std::shared_ptr<TexturedSprite> pTextureSprite = std::make_shared<TexturedSprite>("intro", 0.f, 0.f, 0.2f, 0.2f);
	std::shared_ptr<TextSprite> pTextSprite1 = std::make_shared<TextSprite>("abcABC123:", 0.f, 0.2f, 0.5f, 0.5f);
	std::shared_ptr<TextSprite> pTextSprite2 = std::make_shared<TextSprite>("3D Game Programming", 0.0f, 0.5f, 1.f, 0.8f, XMFLOAT4(1,0,0,1));

	m_pSprites.push_back(pTextureSprite);
	m_pSprites.push_back(pTextSprite1);
	m_pSprites.push_back(pTextSprite2);


	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void Scene::ReleaseUploadBuffers()
{
	if (m_pPlayer) {
		m_pPlayer->ReleaseUploadBuffers();
	}

	for (auto& pObj : m_pGameObjects) {
		pObj->ReleaseUploadBuffers();
	}

}

bool Scene::ProcessInput(UCHAR* pKeysBuffer)
{
	return false;
}

void Scene::Update(float fTimeElapsed)
{
	if (m_pPlayer) {
		m_pPlayer->Update(fTimeElapsed);
	}
	
	for (auto& pObj : m_pGameObjects) {
		pObj->Update(fTimeElapsed);
	}

}

void Scene::Render(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dRootSignature.Get());
	RENDER->SetDescriptorHeapToPipeline(pd3dCommandList);
	UpdateShaderVariable(pd3dCommandList);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dCPUHandle = RENDER->GetDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();

	// Per Scene Descriptor ¿¡ º¹»ç
	auto pCamera = m_pPlayer->GetCamera();
	pCamera->UpdateShaderVariables(pd3dCommandList);
	pCamera->SetViewportsAndScissorRects(pd3dCommandList);

	pd3dDevice->CopyDescriptorsSimple(1, d3dCPUHandle, 
		pCamera->GetCBuffer().GetCPUDescriptorHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	d3dCPUHandle.ptr += GameFramework::g_uiDescriptorHandleIncrementSize;

	pd3dDevice->CopyDescriptorsSimple(1, d3dCPUHandle, 
		m_LightCBuffer.GetCPUDescriptorHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	pd3dCommandList->SetGraphicsRootDescriptorTable(0, RENDER->GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());

	if (m_pPlayer) {
		m_pPlayer->UpdateTransform();
		m_pPlayer->AddToRenderMap();
	}

	for (auto& pObj : m_pGameObjects) {
		pObj->UpdateTransform();
		pObj->AddToRenderMap();
	}

	for (auto& pSprite : m_pSprites) {
		pSprite->AddToUI(0);
	}
}

void Scene::CreateShaderVariables(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	m_LightCBuffer.Create(pd3dDevice, pd3dCommandList, ConstantBufferSize<CB_SCENE_DATA>::value, true);

	for (auto& pSprite : m_pSprites) {
		pSprite->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}
}

void Scene::UpdateShaderVariable(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	CB_SCENE_DATA data;
	data.nLights = m_pLights.size();

	for (int i = 0; i < m_pLights.size(); ++i) {
		data.LightData[i] = m_pLights[i]->MakeLightData();
	}

	data.globalAmbientLight = m_xmf4GlobalAmbient;

	m_LightCBuffer.UpdateData(&data);
}

bool Scene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	POINT ptClicked;
	ptClicked.x = HIWORD(lParam);
	ptClicked.y = LOWORD(lParam);


	return false;
}

bool Scene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'W': m_pPlayer->MoveForward(+1.0f); break;
		case 'S': m_pPlayer->MoveForward(-1.0f); break;
		case 'A': m_pPlayer->MoveStrafe(-1.0f); break;
		case 'D': m_pPlayer->MoveStrafe(+1.0f); break;
		case 'Q': m_pPlayer->MoveUp(+1.0f); break;
		case 'R': m_pPlayer->MoveUp(-1.0f); break;

		default:
			break;
		}
		break;
	default:
		break;
	}
	return false;
}

void Scene::CreateRootSignature(ComPtr<ID3D12Device> pd3dDevice)
{
	D3D12_DESCRIPTOR_RANGE d3dPerSceneDescriptorRange{};
	{
		d3dPerSceneDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		d3dPerSceneDescriptorRange.NumDescriptors = 2;
		d3dPerSceneDescriptorRange.BaseShaderRegister = 0;		// c0, c1 : Camera, Lights
		d3dPerSceneDescriptorRange.RegisterSpace = 0;
		d3dPerSceneDescriptorRange.OffsetInDescriptorsFromTableStart = 0;
	}

	D3D12_DESCRIPTOR_RANGE d3dMaterialDescriptorRange{};
	{
		d3dMaterialDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		d3dMaterialDescriptorRange.NumDescriptors = 1;
		d3dMaterialDescriptorRange.BaseShaderRegister = 2;		// c2 : Material
		d3dMaterialDescriptorRange.RegisterSpace = 0;
		d3dMaterialDescriptorRange.OffsetInDescriptorsFromTableStart = 0;
	}
	
#ifdef INSTANCING_USING_DESCRIPTOR_TABLE
	D3D12_DESCRIPTOR_RANGE d3dInstancingDataDescriptorRange{};
	{
		d3dInstancingDataDescriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		d3dInstancingDataDescriptorRange.NumDescriptors = 1;
		d3dInstancingDataDescriptorRange.BaseShaderRegister = 0;	//	t0 : Texture
		d3dInstancingDataDescriptorRange.RegisterSpace = 0;
		d3dInstancingDataDescriptorRange.OffsetInDescriptorsFromTableStart = 0;
	}
#endif

	D3D12_ROOT_PARAMETER d3dRootParameters[4];
	{
		// Per Scene
		d3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		d3dRootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
		d3dRootParameters[0].DescriptorTable.pDescriptorRanges = &d3dPerSceneDescriptorRange;
		d3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Material
		d3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		d3dRootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
		d3dRootParameters[1].DescriptorTable.pDescriptorRanges = &d3dMaterialDescriptorRange;
		d3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		// Instance data
#ifdef INSTANCING_USING_DESCRIPTOR_TABLE
		d3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		d3dRootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
		d3dRootParameters[2].DescriptorTable.pDescriptorRanges = &d3dInstancingDataDescriptorRange;
		d3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

#else
		d3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		d3dRootParameters[2].Descriptor.ShaderRegister = 0;
		d3dRootParameters[2].Descriptor.RegisterSpace = 0;
		d3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

#endif // INSTANCING_USING_DESCRIPTOR_TABLE
		
		// Instance count
		d3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		d3dRootParameters[3].Constants.Num32BitValues = 1;
		d3dRootParameters[3].Constants.ShaderRegister = 3;
		d3dRootParameters[3].Constants.RegisterSpace = 0;
		d3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
		| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc{};
	{
		d3dRootSignatureDesc.NumParameters = _countof(d3dRootParameters);
		d3dRootSignatureDesc.pParameters = d3dRootParameters;
		d3dRootSignatureDesc.NumStaticSamplers = 0;
		d3dRootSignatureDesc.pStaticSamplers = NULL;
		d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;
	}

	ComPtr<ID3DBlob> pd3dSignatureBlob = nullptr;
	ComPtr<ID3DBlob> pd3dErrorBlob = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, pd3dSignatureBlob.GetAddressOf(), pd3dErrorBlob.GetAddressOf());
	if (pd3dErrorBlob) {
		char* pErrorString = (char*)pd3dErrorBlob->GetBufferPointer();
		HWND hWnd = ::GetActiveWindow();
		MessageBoxA(hWnd, pErrorString, NULL, 0);
		OutputDebugStringA(pErrorString);
		__debugbreak();
	}


	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), IID_PPV_ARGS(m_pd3dRootSignature.GetAddressOf()));
}

std::shared_ptr<Camera> Scene::GetCamera() const
{
	return m_pPlayer->GetCamera();
}

std::shared_ptr<Player> Scene::GetPlayer() const
{
	return m_pPlayer;
}
