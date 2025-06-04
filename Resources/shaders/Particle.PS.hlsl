#include "Particle.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
};

struct DirectionLight
{
    float4 color;
    float3 direction;
    float intensity;
};



ConstantBuffer<Material> gMaterial : register(b0);

//ConstantBuffer<DirectionLight> gDirectionalLight : register(b1);

Texture2D<float4> gTexture : register(t0);

SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    output.color.rgb = gMaterial.color.rgb * textureColor.rgb * input.color.rgb;
  
    output.color.a = input.color.a;
    
    if (output.color.a == 0.0)
    {
        discard;
    }
    
    return output;
}