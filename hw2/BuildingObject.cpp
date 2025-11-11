#include "stdafx.h"
#include "BuildingObject.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MirrorObject 

MirrorObject::MirrorObject()
{
}

MirrorObject::~MirrorObject()
{
}

void MirrorObject::Initialize()
{
	// 公炼扒 SetChild 捞饶俊 荐青
	UpdateTransform(&m_pParent->GetWorldMatrix());
	XMStoreFloat4(&m_xmf4MirrorPlane, XMPlaneFromPointNormal(XMLoadFloat3(&GetPosition()), XMLoadFloat3(&GetLook())));
}

void MirrorObject::AddToRenderMap()
{
	RENDER->AddMirror(static_pointer_cast<MirrorObject>(shared_from_this()));
}

void MirrorObject::Render(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, DescriptorHandle& refDescHandle)
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// BuildingObject 

BuildingObject::BuildingObject()
{
}

BuildingObject::~BuildingObject()
{
}

void BuildingObject::Initialize(ComPtr<ID3D12Device> pd3dDevice, ComPtr<ID3D12GraphicsCommandList> pd3dCommandList, float fWidth, float fLength, float fHeight, int nWindowsInWidth, int nWindowsInHeights)
{
	//
	//             +y
	//	          |  +z
	//	          |  /
	//	  fHeight | /  fLength
	//	          |/  
	//	          +--------- +x
	//              fWidth

	std::shared_ptr<StandardMesh> pFrontBackMirrorMesh= StandardMesh::GenerateMirrorMesh(pd3dDevice, pd3dCommandList, fWidth, fHeight, nWindowsInWidth, nWindowsInHeights);
	std::shared_ptr<StandardMesh> pLeftRightMirrorMesh = StandardMesh::GenerateMirrorMesh(pd3dDevice, pd3dCommandList, fLength, fHeight, nWindowsInWidth, nWindowsInHeights);

	std::shared_ptr<Material> pMaterial = std::make_shared<Material>(pd3dDevice, pd3dCommandList);
	pMaterial->SetTexture(TEXTURE_INDEX_ALBEDO_MAP, TEXTURE->GetTexture("window"));
	pMaterial->SetMaterialType(MATERIAL_TYPE_ALBEDO_MAP);
	pMaterial->SetShader(SHADER->Get<StandardShader>());

	float fBlendFactor = 0.3f;
	float fHalfWidth = fWidth / 2;
	float fHalfLength = fLength / 2;
	float fHalfHeight = fHeight / 2;
	XMFLOAT3 xmf3AxisY = XMFLOAT3(0.f, 1.f, 0.f);

	// -Z 规氢
	std::shared_ptr<MirrorObject> pMirrorFront = std::make_shared<MirrorObject>();
	pMirrorFront->SetName("Mirror_Front");
	pMirrorFront->SetMesh(pFrontBackMirrorMesh);
	pMirrorFront->SetMaterial(0, pMaterial);
	pMirrorFront->SetBlendFactor(fBlendFactor);
	pMirrorFront->SetPosition(XMFLOAT3(0.f, 0.f, -fHalfLength));
	SetChild(pMirrorFront);
	pMirrorFront->Initialize();	// Plane 积己

	// +Z 规氢
	std::shared_ptr<MirrorObject> pMirrorBack = std::make_shared<MirrorObject>();
	pMirrorBack->SetName("Mirror_Back");
	pMirrorBack->SetMesh(pFrontBackMirrorMesh);
	pMirrorBack->SetMaterial(0, pMaterial);
	pMirrorBack->SetBlendFactor(fBlendFactor);
	pMirrorBack->SetPosition(XMFLOAT3(0.f, 0.f, fHalfLength));
	pMirrorBack->Rotate(&xmf3AxisY, 180.f);
	SetChild(pMirrorBack);
	pMirrorBack->Initialize();	// Plane 积己

	// -X 规氢
	std::shared_ptr<MirrorObject> pMirrorLeft = std::make_shared<MirrorObject>();
	pMirrorLeft->SetName("Mirror_Left");
	pMirrorLeft->SetMesh(pLeftRightMirrorMesh);
	pMirrorLeft->SetMaterial(0, pMaterial);
	pMirrorLeft->SetBlendFactor(fBlendFactor);
	pMirrorLeft->SetPosition(XMFLOAT3(0.f, 0.f, -fHalfWidth));
	pMirrorLeft->Rotate(&xmf3AxisY, -90.f);
	SetChild(pMirrorLeft);
	pMirrorLeft->Initialize();	// Plane 积己

	// +X 规氢
	std::shared_ptr<MirrorObject> pMirrorRight = std::make_shared<MirrorObject>();
	pMirrorRight->SetName("Mirror_Right");
	pMirrorRight->SetMesh(pLeftRightMirrorMesh);
	pMirrorRight->SetMaterial(0, pMaterial);
	pMirrorRight->SetBlendFactor(fBlendFactor);
	pMirrorRight->SetPosition(XMFLOAT3(0.f, 0.f, -fHalfWidth));
	pMirrorRight->Rotate(&xmf3AxisY, 90.f);
	SetChild(pMirrorRight);
	pMirrorRight->Initialize();	// Plane 积己

	// 捞力 堆脖阑 父甸绢具 窃
	std::shared_ptr<StandardMesh> pTopMesh = StandardMesh::GenerateBuildingTopMesh(pd3dDevice, pd3dCommandList, fWidth, fLength);
	std::shared_ptr<GameObject> pTopObject = std::make_shared<GameObject>();
	pTopObject->SetName("Building_Top");
	pTopObject->SetMesh(pTopMesh);
	pTopObject->SetMaterial(0, pMaterial);
	pTopObject->SetPosition(XMFLOAT3(0.f, fHeight, 0.f));
	SetChild(pTopObject);
	pTopObject->Initialize();	// Plane 积己

	// m_xmOBB
	m_xmOBB.Center = XMFLOAT3(0.f, fHalfHeight, 0.f);
	m_xmOBB.Extents = XMFLOAT3(fHalfWidth, fHalfHeight, fHalfLength);

}

