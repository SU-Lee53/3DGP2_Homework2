#include "stdafx.h"
#include "GameScene.h"

GameScene::GameScene()
{
}

void GameScene::BuildDefaultLightsAndMaterials()
{
}

void GameScene::BuildObjects(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
}

void GameScene::ReleaseUploadBuffers()
{
}

bool GameScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return false;
}

void GameScene::Update(float fTimeElapsed)
{
}

void GameScene::Render(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
}

void GameScene::CreateShaderVariables(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
}

void GameScene::UpdateShaderVariable(ComPtr<ID3D12GraphicsCommandList> pd3dCommandList)
{
}

void GameScene::CreateRootSignature(ComPtr<ID3D12Device> pd3dDevice)
{
}

bool GameScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool GameScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}
