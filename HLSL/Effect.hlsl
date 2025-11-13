
cbuffer cbParticleData : register(b0)
{
    float4x4 gmtxViewProjection;
    float3 gvPosition;
    float gfTotalTime;
    float3 gvForce;
    float gfTotalLifetime;
};

struct VS_PARTICLE_INPUT
{
    float3 xmf3Position         : POSITION;
    float4 xmf4Color            : COLOR;
    float3 xmf3InitialVelocity  : VELOSITY;
    float2 xmf2InitialSize      : SIZE;
    float fRandomValue          : RANDOM;
    float fStartTime            : STARTTIME;
    float fLifeTime             : LIFETIME;
    float fMass                 : MASS;
};

struct VS_PARTICLE_OUTPUT
{
    float3 xmf3Position         : POSITION;
    float4 xmf4Color            : COLOR;
    float3 xmf3InitialVelocity  : VELOSITY;
    float2 xmf2InitialSize      : SIZE;
    float fRandomValue          : RANDOM;
    float fStartTime            : STARTTIME;
    float fLifeTime             : LIFETIME;
    float fMass                 : MASS;
};

struct GS_PARTICLE_OUTPUT
{
    float3 xmf3Position : SV_POSITION;
    float4 xmf4Color : COLOR;
    float2 xmf2TexCoord : TEXCOORD;
};

VS_PARTICLE_OUTPUT VSExplosion(VS_PARTICLE_INPUT input)
{
}

[maxvertexcount(4)]
void GSExplosion(point VS_PARTICLE_INPUT input[1], inout TriangleStream<GS_PARTICLE_OUTPUT> outStream)
{
}

float4 PSExplosion(GS_PARTICLE_OUTPUT input)
{
}