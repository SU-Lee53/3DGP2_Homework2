#include "Common.hlsl"
#include "Light.hlsl"

//#define _WITH_VERTEX_LIGHTING

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

float3 ComputeNormal(float3 normal, float3 tangent)
{
	
}

float4 VSStandard(VS_STANDARD_OUTPUT input) : SV_TARGET
{
    float4 cDiffuse;
    float4 cSpecular;
    float4 cEmissive;
    float4 cMetallic;
    
    if()
	
	
}

