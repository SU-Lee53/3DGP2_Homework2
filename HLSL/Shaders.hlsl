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
    float4 cAlbedoColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cSpecularColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cNormalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cMetallicColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 cEmissionColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

    if (gMaterial.m_textureMask & MATERIAL_TYPE_ALBEDO_MAP)
        cAlbedoColor = gtxtAlbedoMap.Sample(gssWrap, input.uv);
    if (gMaterial.m_textureMask & MATERIAL_TYPE_SPECULAR_MAP)
        cSpecularColor = gtxtSpecularMap.Sample(gssWrap, input.uv);
    if (gMaterial.m_textureMask & MATERIAL_TYPE_NORMAL_MAP)
        cNormalColor = gtxtNormalMap.Sample(gssWrap, input.uv);
    if (gMaterial.m_textureMask & MATERIAL_TYPE_METALLIC_MAP)
        cMetallicColor = gtxtMetallicMap.Sample(gssWrap, input.uv);
    if (gMaterial.m_textureMask & MATERIAL_TYPE_EMISSION_MAP)
        cEmissionColor = gtxtEmissionMap.Sample(gssWrap, input.uv);

    float4 cIllumination = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 cColor = cAlbedoColor + cSpecularColor + cEmissionColor;
    if (gMaterial.m_textureMask & MATERIAL_TYPE_NORMAL_MAP)
    {
        float3 normalW = input.normalW;
        float3x3 TBN = float3x3(normalize(input.tangentW), normalize(input.biTangentW), normalize(input.normalW));
        float3 vNormal = normalize(cNormalColor.rgb * 2.0f - 1.0f); //[0, 1] ¡æ [-1, 1]
        normalW = normalize(mul(vNormal, TBN));
        cIllumination = Lighting(input.positionW, normalW);
        cColor = lerp(cColor, cIllumination, 0.5f);
    }

    return cColor;
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
    
    float3 positionW = mul(float4(input.position, 1.f), gmtxTerrainWorld).xyz;
    output.position = mul(float4(positionW, 1.f), mtxViewProjection);
	
    output.color = input.color;
    output.uv0 = input.uv0;
    output.uv1 = input.uv1;
	
    return output;
    
}

float4 PSTerrain(VS_TERRAIN_OUTPUT input) : SV_TARGET0
{
    float4 cBaseColor = gtxtAlbedoMap.Sample(gssWrap, input.uv0 + gmtxTerrainUVOffset);
    float4 cDetailColor = gtxtDetailAlbedoMap.Sample(gssWrap, input.uv1 + gmtxTerrainUVOffset);
    
    float4 cFinalColor = lerp(cBaseColor, cDetailColor, 0.5);
    
    return cFinalColor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OBBDebugShader

struct VS_BILLBOARD_OUTPUT
{
    float4 position : POSITION;
};

struct GS_BILLBOARD_OUTPUT
{
    float4 position : SV_Position;
};

VS_BILLBOARD_OUTPUT VSBillboard(uint nVertexID : SV_VertexID)
{
    VS_BILLBOARD_OUTPUT output;
    output.position = float4(0.f, 0.f, 0.f, 1.f);
    return output;
}

[maxvertexcount(24)]
void GSBillboard(point VS_BILLBOARD_OUTPUT input[1], inout TriangleStream<GS_BILLBOARD_OUTPUT> outStream)
{
}

float4 PSBillboard(GS_BILLBOARD_OUTPUT input) : SV_Target0
{
    return float4(1.f, 0.f, 0.f, 0.f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OBBDebugShader

struct VS_DEBUG_OUTPUT
{
    float4 position : POSITION;
};

struct GS_DEBUG_OUTPUT
{
    float4 position : SV_Position;
};

VS_DEBUG_OUTPUT VSDebug(uint nVertexID : SV_VertexID)
{
    VS_DEBUG_OUTPUT output;
    output.position = float4(0.f, 0.f, 0.f, 1.f);
    return output;
}

float3 RotateByQuaternion(float3 vPosition, float4 Quaternion)
{
    float3 t = 2.0f * cross(Quaternion.xyz, vPosition);
    return vPosition + Quaternion.w * t + cross(Quaternion.xyz, t);
}

[maxvertexcount(24)]
void GSDebug(point VS_DEBUG_OUTPUT input[1], inout TriangleStream<GS_DEBUG_OUTPUT> outStream)
{
    float3 vCenter = gvOBBCenter;
    float3 vExtent = gvOBBExtent;
    float4 qOrientation = normalize(gvOBBOrientationQuat);
    
    float3 vAxisX = RotateByQuaternion(float3(1, 0, 0), qOrientation);
    float3 vAxisY = RotateByQuaternion(float3(0, 1, 0), qOrientation);
    float3 vAxisZ = RotateByQuaternion(float3(0, 0, 1), qOrientation);
    
    float3 ex = vAxisX * vExtent.x;
    float3 ey = vAxisY * vExtent.y;
    float3 ez = vAxisZ * vExtent.z;

    float3 c[8];
    float3 T00 = vCenter + - ex + ey + ez;
    float3 T01 = vCenter + + ex + ey + ez;
    float3 T10 = vCenter + - ex + ey - ez;
    float3 T11 = vCenter + + ex + ey - ez;
    
    float3 B00 = vCenter + - ex - ey + ez;
    float3 B01 = vCenter + + ex - ey + ez;
    float3 B10 = vCenter + - ex - ey - ez;
    float3 B11 = vCenter + + ex - ey - ez;
    
    matrix mtxVP = mul(gmtxView, gmtxProjection);
    
    GS_DEBUG_OUTPUT output;
    
    output.position = mul(float4(T11, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(T01, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(B11, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(B01, 1.f), mtxVP); outStream.Append(output);
    outStream.RestartStrip();
    
    output.position = mul(float4(T00, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(T10, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(B00, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(B10, 1.f), mtxVP); outStream.Append(output);
    outStream.RestartStrip();
    
    output.position = mul(float4(T00, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(T01, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(T10, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(T11, 1.f), mtxVP); outStream.Append(output);
    outStream.RestartStrip();
    
    output.position = mul(float4(B00, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(B01, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(B10, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(B11, 1.f), mtxVP); outStream.Append(output);
    outStream.RestartStrip();
    
    output.position = mul(float4(T01, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(T00, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(B01, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(B00, 1.f), mtxVP); outStream.Append(output);
    outStream.RestartStrip();
    
    output.position = mul(float4(T10, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(T11, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(B10, 1.f), mtxVP); outStream.Append(output);
    output.position = mul(float4(B11, 1.f), mtxVP); outStream.Append(output);
    outStream.RestartStrip();
    
}

float4 PSDebug(GS_DEBUG_OUTPUT input) : SV_Target0
{
    return float4(1.f, 0.f, 0.f, 0.f);
}