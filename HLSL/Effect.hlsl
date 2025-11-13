
cbuffer cbCameraData : register(b0)
{
    matrix gmtxView;
    matrix gmtxProjection;
    float3 gvCameraPosition;
};

struct ParticleData
{
    float3 vPosition;
    float fElapsedTime;
    float3 vForce;
    float pad;
};

#define MAX_EFFECT_PER_DRAW 100

cbuffer cbParticleData : register(b1)
{
    ParticleData gParticleData[MAX_EFFECT_PER_DRAW];
};

cbuffer cbParticleData : register(b2)
{
    uint gnDataIndex;
};

struct VS_PARTICLE_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
    float3 initialVelocity : VELOCITY;
    float2 initialSize : SIZE;
    float randomValue : RANDOM;
    float startTime : STARTTIME;
    float lifeTime : LIFETIME;
    float mass : MASS;
};

struct VS_PARTICLE_OUTPUT
{
    float3 positionW : POSITION;
    float4 color : COLOR;
    float3 initialVelocity : VELOCITY;
    float2 initialSize : SIZE;
    float randomValue : RANDOM;
    float startTime : STARTTIME;
    float lifeTime : LIFETIME;
    float mass : MASS;
    uint instanceID : INSTANCEID;
};

struct GS_PARTICLE_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

const static float3 gvGravity = float3(0.f, -9.8f, 0.f);

VS_PARTICLE_OUTPUT VSExplosion(VS_PARTICLE_INPUT input, uint nInstanceID : SV_InstanceID)
{
    VS_PARTICLE_OUTPUT output;
    output.positionW = input.position + gParticleData[gnDataIndex + nInstanceID].vPosition;
    output.color = input.color;
    output.initialVelocity = input.initialVelocity;
    output.initialSize = input.initialSize;
    output.randomValue = input.randomValue;
    output.startTime = input.startTime;
    output.lifeTime = input.lifeTime;
    output.mass = input.mass;
    output.instanceID = nInstanceID;
    return output;
}

[maxvertexcount(4)]
void GSExplosion(point VS_PARTICLE_OUTPUT input[1], inout TriangleStream<GS_PARTICLE_OUTPUT> outStream)
{
    float fElapsedTime = gParticleData[gnDataIndex + input[0].instanceID].fElapsedTime;
    float3 vForce = gParticleData[gnDataIndex + input[0].instanceID].vForce;
    
    
    float3 vUp = gmtxView._12_22_32;
    //float3 vUp = float3(0, 1, 0);
    float3 vLook = gvCameraPosition.xyz - input[0].positionW;
    vLook = normalize(vLook);
    float3 vRight = cross(vUp, vLook);
    
    float3 vNewPosition = input[0].positionW;
    float4 cNewColor = float4(0.f, 0.f, 0.f, 0.f);
    float fNewAlpha = 0.f;
    float2 vNewSize = float2(0, 0);
    
    if (fElapsedTime <= input[0].lifeTime)
    {
        float fNewTime = frac(fElapsedTime / input[0].lifeTime) * input[0].lifeTime; // 0 ~ lifeTime
        float fNewTimeSq = fNewTime * fNewTime;
    
        float fOneMinusTime = 1 - frac(fElapsedTime / input[0].lifeTime); // 1 ~ 0
    
        // 새로운 위치의 계산
        float fForceX = vForce.x + (gvGravity.x * 20.f) * input[0].mass;
        float fForceY = vForce.y + (gvGravity.y * 20.f) * input[0].mass;
        float fForceZ = vForce.z + (gvGravity.z * 20.f) * input[0].mass;
    
        // F = ma -> a = F / m
        float fAccX = fForceX / input[0].mass;
        float fAccY = fForceY / input[0].mass;
        float fAccZ = fForceZ / input[0].mass;
    
        // s = v0t * 1/2at^2
        float3 initialDirection = normalize(input[0].initialVelocity);
        float dX = (initialDirection.x * (input[0].randomValue * 250.f) * fNewTime) + (0.5 * fAccX * fNewTimeSq);
        float dY = (initialDirection.y * (input[0].randomValue * 250.f) * fNewTime) + (0.5 * fAccY * fNewTimeSq);
        float dZ = (initialDirection.z * (input[0].randomValue * 250.f) * fNewTime) + (0.5 * fAccZ * fNewTimeSq);
    
        vNewPosition += float3(dX, dY, dZ);
    
        // 알파값 계산 -> 시간이 지날수록 투명
        fNewAlpha = 1 - frac(fElapsedTime / input[0].lifeTime); // 1 ~ 0
        cNewColor = float4(input[0].color.xyz, fNewAlpha);
    
        // 크기 계산 -> 시간이 지날수록 감소
        vNewSize = float2(input[0].initialSize.x * fNewAlpha, input[0].initialSize.y * fNewAlpha);
    }
    else
    {
        // lifeTime 이 지나면 저멀리 이상한곳으로 보냄
        vNewPosition += float3(9999999.f, 9999999.f, 9999999.f);
    }
    
    // 출력
    float4 vertices[4];
    vertices[0] = float4(vNewPosition + (vNewSize.x * vRight) - (vNewSize.y * vUp), 1.f);
    vertices[1] = float4(vNewPosition + (vNewSize.x * vRight) + (vNewSize.y * vUp), 1.f);
    vertices[2] = float4(vNewPosition - (vNewSize.x * vRight) - (vNewSize.y * vUp), 1.f);
    vertices[3] = float4(vNewPosition - (vNewSize.x * vRight) + (vNewSize.y * vUp), 1.f);
    
    float2 uvs[4] = { float2(0.f, 1.f), float2(0.f, 0.f), float2(1.f, 1.f), float2(1.f, 0.f) };
    
    float4x4 mtxViewProjection = mul(gmtxView, gmtxProjection);
    GS_PARTICLE_OUTPUT output;
    
    output.position = mul(vertices[0], mtxViewProjection);
    output.color = cNewColor;
    output.uv = uvs[0];
    outStream.Append(output);
    
    output.position = mul(vertices[1], mtxViewProjection);
    output.color = cNewColor;
    output.uv = uvs[1];
    outStream.Append(output);
    
    output.position = mul(vertices[2], mtxViewProjection);
    output.color = cNewColor;
    output.uv = uvs[2];
    outStream.Append(output);
    
    output.position = mul(vertices[3], mtxViewProjection);
    output.color = cNewColor;
    output.uv = uvs[3];
    outStream.Append(output);
}

float4 PSExplosion(GS_PARTICLE_OUTPUT input) : SV_Target0
{
    return input.color;
}