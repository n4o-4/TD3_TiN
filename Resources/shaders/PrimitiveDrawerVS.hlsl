#include "PrimitiveDrawer.hlsli"

struct LineForGPU
{
    float4x4 matWorld; // 4x4 �̃��[���h�s��
    float4 color; // ���̐F (Vector4 �^)
};

struct ViewProjection
{
    float3 worldPosition;
    float4x4 matViewProjection;
};

ConstantBuffer<LineForGPU> gLineForGPU : register(b0);

ConstantBuffer<ViewProjection> gViewProjection : register(b1);

struct VertexShaderInput
{
    float4 position : POSITION0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    float4x4 WVP = mul(gLineForGPU.matWorld, gViewProjection.matViewProjection);
    
    output.position = mul(input.position, WVP);
    
    output.color = gLineForGPU.color;
    
    return output;
}