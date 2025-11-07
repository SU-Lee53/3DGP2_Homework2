#include "stdafx.h"
#include "Scene.h"
#include "TerrainObject.h"

Scene::Scene()
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


	GameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dRootSignature, "../Models/SuperCobra.bin");

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

	std::shared_ptr<TexturedSprite> pTextureSprite = std::make_shared<TexturedSprite>("intro", 0.f, 0.f, 1.0f, 1.0f);
	std::shared_ptr<TextSprite> pTextSprite1 = std::make_shared<TextSprite>("abcABC123:", 0.f, 0.2f, 0.5f, 0.5f, XMFLOAT4(1, 1, 1, 1), 1, true);
	std::shared_ptr<TextSprite> pTextSprite2 = std::make_shared<TextSprite>("3D Game Programming", 0.0f, 0.5f, 1.f, 0.8f, XMFLOAT4(1,0,0,1), 1, true);

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

	if (m_pTerrain) {
		m_pTerrain->Update(fTimeElapsed);
	}
}

void Scene::Render(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
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

	if (m_pTerrain) {
		RENDER->SetTerrain(m_pTerrain);
	}
}

void Scene::RenderDebug(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	if (m_pPlayer) {
		m_pPlayer->RenderOBB(pd3dCommandList);
	}

	for (auto& pObj : m_pGameObjects) {
		pObj->RenderOBB(pd3dCommandList);
	}

}

void Scene::CreateShaderVariables(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	m_LightCBuffer.Create(pd3dDevice, pd3dCommandList, ConstantBufferSize<CB_LIGHT_DATA>::value, true);

	for (auto& pSprite : m_pSprites) {
		pSprite->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	}
}

void Scene::UpdateShaderVariable(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	CB_LIGHT_DATA lightData;
	lightData.nLights = m_pLights.size();

	for (int i = 0; i < m_pLights.size(); ++i) {
		lightData.LightData[i] = m_pLights[i]->MakeLightData();
	}

	lightData.globalAmbientLight = m_xmf4GlobalAmbient;

	m_LightCBuffer.UpdateData(&lightData);

}

bool Scene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
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
}

std::shared_ptr<Sprite> Scene::CheckButtonClicked()
{
	POINT ptCursorPos;
	GetCursorPos(&ptCursorPos);
	ScreenToClient(GameFramework::g_hWnd, &ptCursorPos);

	float fCursorX = (float)ptCursorPos.x / GameFramework::g_nClientWidth;
	float fCursorY = (float)ptCursorPos.y / GameFramework::g_nClientHeight;

	std::shared_ptr<Sprite> pReturn = nullptr;
	int nLastClickedSpriteLayerIndex = -1;
	for (const auto& pSprite : m_pSprites) {
		if (pSprite->IsCursorInSprite(fCursorX, fCursorY)) {
			pReturn = (int)pSprite->GetLayerIndex() > nLastClickedSpriteLayerIndex ? pSprite : pReturn;
		}
	}

	return pReturn;
}

std::shared_ptr<Camera> Scene::GetCamera() const
{
	return m_pPlayer->GetCamera();
}

std::shared_ptr<Player> Scene::GetPlayer() const
{
	return m_pPlayer;
}
