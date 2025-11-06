#ifndef _COMMON_
#define _COMMON_


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Camera

cbuffer cbCameraData : register(b0)
{
    matrix gmtxView;
    matrix gmtxProjection;
    float3 gvCameraPosition;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Light

#define MAX_LIGHTS			16 
#define MAX_MATERIALS		512 

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3

#define _WITH_LOCAL_VIEWER_HIGHLIGHTING
#define _WITH_THETA_PHI_CONES
//#define _WITH_REFLECT

struct LIGHT
{
    float4 m_cAmbient;
    float4 m_cDiffuse;
    float4 m_cSpecular;
    float3 m_vPosition;
    float m_fFalloff;
    float3 m_vDirection;
    float m_fTheta; //cos(m_fTheta)
    float3 m_vAttenuation;
    float m_fPhi; //cos(m_fPhi)
    bool m_bEnable;
    int m_nType;
    float m_fRange;
    float padding;
};

cbuffer cbLightData : register(b1)
{
    LIGHT gLights[MAX_LIGHTS];
    float4 gcGlobalAmbientLight;
    int gnLights;
};

cbuffer cbTerrainData : register(b2)
{
    matrix gmtxTerrainWorld;
};

Texture2DArray gtxtarrSkybox : register(t0);

SamplerState gssWrap : register(s0);
SamplerState gssClamp : register(s1);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Per Object 

struct MATERIAL
{
    float4 m_cAmbient;
    float4 m_cDiffuse;
    float4 m_cSpecular; //a = power
    float4 m_cEmissive;
    uint m_textureMask;
};

cbuffer cbGameObjectData : register(b3)
{
    MATERIAL gMaterial;
    int gnBaseIndex;
};

Texture2D gtxtAlbedoMap         : register(t2);
Texture2D gtxtSpecularMap       : register(t3);
Texture2D gtxtNormalMap         : register(t4);
Texture2D gtxtMetaillicMap      : register(t5);
Texture2D gtxtEmissionMap       : register(t6);
Texture2D gtxtDetailAlbedoMap   : register(t7);
Texture2D gtxtDetailNormalMap   : register(t8);

#define MATERIAL_TYPE_ALBEDO_MAP           0x01
#define MATERIAL_TYPE_SPECULAR_MAP         0x02
#define MATERIAL_TYPE_NORMAL_MAP           0x04
#define MATERIAL_TYPE_METALLIC_MAP         0x08
#define MATERIAL_TYPE_EMISSION_MAP         0x10
#define MATERIAL_TYPE_DETAIL_ALBEDO_MAP    0x20
#define MATERIAL_TYPE_DETAIL_NORMAL_MAP    0x40

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Instancing Data 

//struct INSTANCE_DATA
//{
//    matrix mtxGameObject;
//};

StructuredBuffer<matrix> sbInstanceData : register(t11);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Debug Data 

cbuffer cbOBBDebugData : register(b4)
{
    float3 gvOBBCenter;
    float3 gvOBBExtent;
    float4 gvOBBOrientationQuat;
};

#endif