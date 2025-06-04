#include "Fullscreen.hlsli"

// —”¶¬ŠÖ”@https://www.ronja-tutorials.com/post/024-white-noise/
float rand2dTo1d(float2 value, float2 dotDir = float2(12.9898, 78.233))
{
    float2 smallValue = sin(value);
    float random = dot(smallValue, dotDir);
    random = frac(sin(random) * 143758.5453);
    return random;
}

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct Material
{
    float time;
};

ConstantBuffer<Material> gMaterial : register(b0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // —”¶¬
    float random = rand2dTo1d(input.texcoord * gMaterial.time);
    
    output.color = float4(random, random, random, 1.0f);
    
    return output;
}