#include "stdafx.h"
#include "IntroScene.h"

IntroScene::IntroScene()
{
}

void IntroScene::BuildDefaultLightsAndMaterials()
{
}

void IntroScene::BuildObjects(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
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

	std::shared_ptr<TexturedSprite> pTextureSprite = std::make_shared<TexturedSprite>("intro", 0.f, 0.f, 1.0f, 1.0f);
	std::shared_ptr<TextSprite> pTextSprite1 = std::make_shared<TextSprite>("3D Game Programming 2", 0.0f, 0.0f, 1.f, 0.2f, XMFLOAT4(1, 0, 0, 1), 1, false);
	std::shared_ptr<TextSprite> pTextSprite2 = std::make_shared<TextSprite>("Start", 0.3f, 0.85f, 0.7f, 0.95f, XMFLOAT4(1, 0, 0, 1), 1, true);

	m_pSprites.push_back(pTextureSprite);
	m_pSprites.push_back(pTextSprite1);
	m_pSprites.push_back(pTextSprite2);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void IntroScene::ReleaseUploadBuffers()
{
	Scene::ReleaseUploadBuffers();
}

bool IntroScene::ProcessInput(UCHAR* pKeysBuffer)
{
	if (pKeysBuffer[VK_LBUTTON] & 0xF0) {
		auto pClickedSprite = CheckButtonClicked();
		if (pClickedSprite == m_pSprites[2]) {
			// TODO : Change Scene
			GameFramework::ChangeScene(1);
		}

	}

	return true;
}

void IntroScene::Update(float fTimeElapsed)
{
	Scene::Update(fTimeElapsed);
}

void IntroScene::Render(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	Scene::Render(pd3dDevice, pd3dCommandList);
}

void IntroScene::CreateShaderVariables(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	Scene::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void IntroScene::UpdateShaderVariable(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
	Scene::UpdateShaderVariable(pd3dCommandList);
}

void IntroScene::CreateRootSignature(ComPtr<ID3D12Device> pd3dDevice)
{
	Scene::CreateRootSignature(pd3dDevice);
}

bool IntroScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool IntroScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}
