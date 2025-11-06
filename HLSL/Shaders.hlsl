#include "Common.hlsl"
#include "Light.hlsl"

//#define _WITH_VERTEX_LIGHTING

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Standard Shader

struct VS_STANDARD_INPUT
{
	float3 position		: POSITION;
    float2 uv			: TEXCOORD0;
	float3 normal		: NORMAL;
    float3 tangent		: TANGENT;
    float3 biTangent	: BITANGENT;
	
};

struct VS_STANDARD_OUTPUT
{
	float4 position		: SV_POSITION;
	float3 positionW	: POSITION;
    float2 uv			: TEXCOORD0;
	float3 normalW		: NORMAL;
	float3 tangentW		: TANGENT;
	float3 biTangentW	: BITANGENT;
};

VS_STANDARD_OUTPUT VSStandard(VS_STANDARD_INPUT input, uint nInstanceID : SV_InstanceID)
{
    VS_STANDARD_OUTPUT output;

    matrix mtxViewProjection = mul(gmtxView, gmtxProjection);
	
    output.positionW = mul(float4(input.position, 1.f), sbInstanceData[gnBaseIndex + nInstanceID]).xyz;
    output.position = mul(float4(output.positionW, 1.f), mtxViewProjection);
	
    output.normalW = mul(float4(input.normal, 0.f), sbInstanceData[gnBaseIndex + nInstanceID]).xyz;
    output.tangentW = mul(float4(input.tangent, 0.f), sbInstanceData[gnBaseIndex + nInstanceID]).xyz;
    output.biTangentW = mul(float4(input.biTangent, 0.f), sbInstanceData[gnBaseIndex + nInstanceID]).xyz;
	
    output.uv = input.uv;
	
	return output;
}

float3 ComputeNormal(float3 normalW, float3 tangentW, float2 uv)
{
    float3 N = normalize(normalW);
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);
    float3x3 TBN = float3x3(T, B, N);
    
    float3 normalFromMap = gtxtNormalMap.Sample(gssWrap, uv).rgb;
    float3 normal = (normalFromMap * 2.0f) - 1.0f; // [0, 1] ---> [-1, 1]
    
    return mul(normal, TBN);
}

float4 PSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
    float4 cColor{};
    float3 normal = input.normalW;
    
    if (gMaterial.m_textureMask & MATERIAL_TYPE_ALBEDO_MAP)
    {
        cColor += gtxtAlbedoMap.Sample(gssWrap, input.uv);
    }
    if (gMaterial.m_textureMask & MATERIAL_TYPE_SPECULAR_MAP)
    {
        cColor += gtxtSpecularMap.Sample(gssWrap, input.uv);
    }
    if (gMaterial.m_textureMask & MATERIAL_TYPE_EMISSION_MAP)
    {
        cColor += gtxtEmissionMap.Sample(gssWrap, input.uv);
    }
    if (gMaterial.m_textureMask & MATERIAL_TYPE_METALLIC_MAP)
    {
        cColor += gtxtMetaillicMap.Sample(gssWrap, input.uv);
    }
    if (gMaterial.m_textureMask & MATERIAL_TYPE_NORMAL_MAP)
    {
        normal = ComputeNormal(input.normalW, input.tangentW, input.uv);
    }
    
    float4 cIllumination = Lighting(input.positionW, input.normalW);
    float4 cFinalColor = lerp(cIllumination, cColor, 0.5);
    
    return cFinalColor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TerrainShader

struct VS_TERRAIN_INPUT
{
    float3 position : POSITION;
    float4 color    : COLOR;
    float2 uv0      : TEXCOORD0;
    float2 uv1      : TEXCOORD1;
};

struct VS_TERRAIN_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
    float2 uv0      : TEXCOORD0;
    float2 uv1      : TEXCOORD1;
};

VS_TERRAIN_OUTPUT VSTerrain(VS_TERRAIN_INPUT input)
{
    VS_TERRAIN_OUTPUT output;
    
    matrix mtxViewProjection = mul(gmtxView, gmtxProjection);
    
    float3 positionW = mul(float4(input.position, 1.f), gmtxTerrainWorld);
    output.position = mul(float4(positionW, 1.f), mtxViewProjection);
	
    output.color = input.color;
    output.uv0 = input.uv0;
    output.uv1 = input.uv1;
	
    return output;
    
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET0
{
    float4 cBaseColor = gtxtAlbedoMap.Sample(gssWrap, input.uv0);
    float4 cDetailColor = gtxtDetailAlbedoMap.Sample(gssWrap, input.uv1);
    
    float4 cFinalColor = lerp(cBaseColor, cDetailColor, 0.5);
    
    return cFinalColor;
}