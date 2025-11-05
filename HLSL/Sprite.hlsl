
struct VS_SPRITE_OUTPUT
{
    float4 pos : SV_POSITION;
    uint nVertexID : INSTANCEID;
};

struct GS_SPRITE_OUTPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

cbuffer SPRITE_DATA : register(b0)
{
    float gnLeft;
    float gnTop;
    float gnRight;
    float gnBottom;
};

Texture2D gtxtFontTexture : register(t0);
Texture2D gtxtSpriteTexture : register(t1);
SamplerState gSamplerState : register(s0);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Texture Sprite
// 화면에 이미지를 그림

VS_SPRITE_OUTPUT VSTextureSprite(uint nVertexID : SV_VertexID)
{
    VS_SPRITE_OUTPUT output;
    
    output.pos = float4(0, 0, 0, 1);
    output.nVertexID = nVertexID;
    
    return output;
}

[maxvertexcount(4)]
void GSTextureSprite(point VS_SPRITE_OUTPUT input[1], inout TriangleStream<GS_SPRITE_OUTPUT> outStream)
{
    float fLeft = gnLeft * 2.f - 1.f;
    float fTop = 1.f - gnTop * 2.f;
    float fRight = gnRight * 2.f - 1.f;
    float fBottom = 1.f - gnBottom * 2.f;
    
    GS_SPRITE_OUTPUT vertex;
    
    vertex.pos = float4(fLeft, fTop, 0.f, 1.f);
    vertex.uv = float2(0.f, 0.f);
    outStream.Append(vertex);
    
    vertex.pos = float4(fRight, fTop, 0.f, 1.f);
    vertex.uv = float2(1.f, 0.f);
    outStream.Append(vertex);
    
    vertex.pos = float4(fLeft, fBottom, 0.f, 1.f);
    vertex.uv = float2(0.f, 1.f);
    outStream.Append(vertex);
    
    vertex.pos = float4(fRight, fBottom, 0.f, 1.f);
    vertex.uv = float2(1.f, 1.f);
    outStream.Append(vertex);
}

float4 PSTextureSprite(GS_SPRITE_OUTPUT input) : SV_Target
{
    return gtxtSpriteTexture.Sample(gSamplerState, input.uv);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Text Sprite
// 화면에 글자를 그림

//#define MAX_CHARACTER_PER_SPRITE 10

cbuffer TEXT_DATA : register(b1)
{
    uint4 gCharacters[10];
    float4 gcTextColor;
    uint gnLength;
}

VS_SPRITE_OUTPUT VSTextSprite(uint nVertexID : SV_VertexID)
{
    VS_SPRITE_OUTPUT output;
    
    output.pos = float4(0, 0, 0, 1);
    output.nVertexID = nVertexID; // text 길이만큼 인스턴싱으로 그림 -> gCharacter[nInstanceID - 1] 가 현재 그리려는 글자임
    
    return output;
}

[maxvertexcount(4)]
void GSTextSprite(point VS_SPRITE_OUTPUT input[1], inout TriangleStream<GS_SPRITE_OUTPUT> outStream)
{
    float fRectLeft = gnLeft * 2.f - 1.f;
    float fRectTop = 1.f - gnTop * 2.f;
    float fRectRight = gnRight * 2.f - 1.f;
    float fRectBottom = 1.f - gnBottom * 2.f;
    
    float nFontRectWidth = abs(fRectRight - fRectLeft) / gnLength;
    float nFontRectHeight = abs(fRectBottom - fRectTop);
    
    float fLeft = fRectLeft + (nFontRectWidth * input[0].nVertexID);
    float fTop = fRectTop;
    float fRight = fLeft + nFontRectWidth;
    float fBottom = fRectBottom;
    
    float2 ltuv = float2(0, 0);
    float2 rtuv = float2(0, 0);
    float2 lbuv = float2(0, 0);
    float2 rbuv = float2(0, 0);
    
    float fontStrideU = 1.0f / 26.f;
    float fontStrideV = 1.0f / 3.f;
    
    uint nCharacter = gCharacters[(input[0].nVertexID) / 4][(input[0].nVertexID) % 4];
    if (nCharacter >= 65 && nCharacter <= 90) {
        // 대문자
        uint nCharacterStride = nCharacter - 65;
        
        ltuv = float2(fontStrideU * nCharacterStride, 0);
        rtuv = float2(fontStrideU * (nCharacterStride + 1), 0);
        lbuv = float2(fontStrideU * nCharacterStride, fontStrideV);
        rbuv = float2(fontStrideU * (nCharacterStride + 1), fontStrideV);
    }
    else if (nCharacter >= 97 && nCharacter <= 122) {
        // 소문자
        uint nCharacterStride = nCharacter - 97;
        
        ltuv = float2(fontStrideU * nCharacterStride, fontStrideV * 1);
        rtuv = float2(fontStrideU * (nCharacterStride + 1), fontStrideV * 1);
        lbuv = float2(fontStrideU * nCharacterStride, fontStrideV * 2);
        rbuv = float2(fontStrideU * (nCharacterStride + 1), fontStrideV * 2);
    }
    else if (nCharacter >= 48 && nCharacter <= 57) {
        // 숫자
        uint nCharacterStride = nCharacter - 48;
        
        ltuv = float2(fontStrideU * nCharacterStride, fontStrideV * 2);
        rtuv = float2(fontStrideU * (nCharacterStride + 1), fontStrideV * 2);
        lbuv = float2(fontStrideU * nCharacterStride, fontStrideV * 3);
        rbuv = float2(fontStrideU * (nCharacterStride + 1), fontStrideV * 3);
    }
    else if (nCharacter == 58) {
        // 콜론(:)
        uint nCharacterStride = 10;
        
        ltuv = float2(fontStrideU * nCharacterStride, fontStrideV * 2);
        rtuv = float2(fontStrideU * (nCharacterStride + 1), fontStrideV * 2);
        lbuv = float2(fontStrideU * nCharacterStride, fontStrideV * 3);
        rbuv = float2(fontStrideU * (nCharacterStride + 1), fontStrideV * 3);
    }
    
    GS_SPRITE_OUTPUT vertex;
    
    vertex.pos = float4(fLeft, fTop, 0.f, 1.f);
    vertex.uv = ltuv;
    outStream.Append(vertex);
    
    vertex.pos = float4(fRight, fTop, 0.f, 1.f);
    vertex.uv = rtuv;
    outStream.Append(vertex);
    
    vertex.pos = float4(fLeft, fBottom, 0.f, 1.f);
    vertex.uv = lbuv;
    outStream.Append(vertex);
    
    vertex.pos = float4(fRight, fBottom, 0.f, 1.f);
    vertex.uv = rbuv;
    outStream.Append(vertex);
}

float4 PSTextSprite(GS_SPRITE_OUTPUT input) : SV_Target
{
    return gtxtFontTexture.Sample(gSamplerState, input.uv) * gcTextColor;
}
