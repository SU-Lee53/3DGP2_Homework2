#include "stdafx.h"
#include "GameScene.h"
#include "TerrainObject.h"

GameScene::GameScene()
{
}

void GameScene::BuildDefaultLightsAndMaterials()
{
	m_pLights.reserve(4);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	std::shared_ptr<PointLight> pLight1 = std::make_shared<PointLight>();
	pLight1->m_bEnable = true;
	pLight1->m_fRange = 1000.0f;
	pLight1->m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	pLight1->m_xmf4Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	pLight1->m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	pLight1->m_xmf3Position = XMFLOAT3(30.0f, 30.0f, 30.0f);
	pLight1->m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	pLight1->m_fAttenuation0 = 1.0f;
	pLight1->m_fAttenuation1 = 0.001f;
	pLight1->m_fAttenuation2 = 0.0001f;
	m_pLights.push_back(pLight1);

	std::shared_ptr<SpotLight> pLight2 = std::make_shared<SpotLight>();
	pLight2->m_bEnable = true;
	pLight2->m_fRange = 500.0f;
	pLight2->m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	pLight2->m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	pLight2->m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	pLight2->m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	pLight2->m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	pLight2->m_fAttenuation0 = 1.0f;
	pLight2->m_fAttenuation1 = 0.01f;
	pLight2->m_fAttenuation2 = 0.0001f;
	pLight2->m_fFalloff = 8.0f;
	pLight2->m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	pLight2->m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	m_pLights.push_back(pLight2);

	std::shared_ptr<DirectionalLight> pLight3 = std::make_shared<DirectionalLight>();
	pLight3->m_bEnable = true;
	pLight3->m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	pLight3->m_xmf4Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	pLight3->m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
	pLight3->m_xmf3Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_pLights.push_back(pLight3);

	std::shared_ptr<SpotLight> pLight4 = std::make_shared<SpotLight>();
	pLight4->m_bEnable = true;
	pLight4->m_fRange = 600.0f;
	pLight4->m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	pLight4->m_xmf4Diffuse = XMFLOAT4(0.3f, 0.7f, 0.0f, 1.0f);
	pLight4->m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	pLight4->m_xmf3Position = XMFLOAT3(50.0f, 30.0f, 30.0f);
	pLight4->m_xmf3Direction = XMFLOAT3(0.0f, 1.0f, 1.0f);
	pLight4->m_fAttenuation0 = 1.0f;
	pLight4->m_fAttenuation1 = 0.01f;
	pLight4->m_fAttenuation2 = 0.0001f;
	pLight4->m_fFalloff = 8.0f;
	pLight4->m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	pLight4->m_fTheta = (float)cos(XMConvertToRadians(30.0f));
	m_pLights.push_back(pLight4);

	//m_xmf4GlobalAmbient = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
}

void GameScene::BuildObjects(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
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

	auto pSuperCobra = RESOURCE->CopyGameObject("SuperCobra");
	pSuperCobra->SetScale(10.0f, 10.0f, 10.0f);
	pAirplanePlayer->SetChild(pSuperCobra);

	m_pPlayer = pAirplanePlayer;
	m_pPlayer->Initialize();

	m_pPlayer->SetFriction(20.5f);
	m_pPlayer->SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_pPlayer->SetMaxVelocityXZ(125.5f);
	m_pPlayer->SetMaxVelocityY(140.0f);
	m_pPlayer->SetPosition(XMFLOAT3(1500.f, 1000.0f, 1500.0f));

	XMFLOAT3 xmf3Scale(18.0f, 4.0f, 18.0f);
	XMFLOAT4 xmf4Color(0.0f, 0.5f, 0.0f, 0.0f);
	m_pTerrain = std::make_shared<TerrainObject>();
	m_pTerrain->Initialize(pd3dDevice, pd3dCommandList, "../Models/Textures/Terrain/HeightMap.raw", 257, 257, 257, 257, xmf3Scale, xmf4Color);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void GameScene::ReleaseUploadBuffers()
{
	Scene::ReleaseUploadBuffers();
}

bool GameScene::ProcessInput(UCHAR* pKeysBuffer)
{
	DWORD dwDirection = 0;
	if (pKeysBuffer['W'] & 0xF0)	dwDirection |= MOVE_DIR_FORWARD;
	if (pKeysBuffer['S'] & 0xF0)	dwDirection |= MOVE_DIR_BACKWARD;
	if (pKeysBuffer['A'] & 0xF0)	dwDirection |= MOVE_DIR_LEFT;
	if (pKeysBuffer['D'] & 0xF0)	dwDirection |= MOVE_DIR_RIGHT;
	if (pKeysBuffer['E'] & 0xF0)	dwDirection |= MOVE_DIR_UP;
	if (pKeysBuffer['Q'] & 0xF0)	dwDirection |= MOVE_DIR_DOWN;

	float cxDelta = 0.0f, cyDelta = 0.0f;
	POINT ptCursorPos;
	if (GetCapture() == GameFramework::g_hWnd) {
		SetCursor(NULL);
		GetCursorPos(&ptCursorPos);
		cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
		cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
		SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
	}

	if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f)) {
		if (cxDelta || cyDelta) {
			if (pKeysBuffer[VK_RBUTTON] & 0xF0)
				m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
			else
				m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
		}
		if (dwDirection) {
			m_pPlayer->Move(dwDirection, 1.5f, true);
		}
	}


	return true;
}

void GameScene::Update(float fTimeElapsed)
{
	if (m_pLights.size() != 0)
	{
		std::shared_ptr<SpotLight> pSpotLight = static_pointer_cast<SpotLight>(m_pLights[1]);
		pSpotLight->m_xmf3Position = m_pPlayer->GetPosition();
		pSpotLight->m_xmf3Direction = m_pPlayer->GetLookVector();
	}

	Scene::Update(fTimeElapsed);
}

void GameScene::Render(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	Scene::Render(pd3dDevice, pd3dCommandList);
}

void GameScene::CreateShaderVariables(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	Scene::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void GameScene::UpdateShaderVariable(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	Scene::UpdateShaderVariable(pd3dCommandList);
}

void GameScene::CreateRootSignature(ComPtr<ID3D12Device> pd3dDevice)
{
	Scene::CreateRootSignature(pd3dDevice);
}

bool GameScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID) {
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}

	return true;
}

bool GameScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}
