#include "Fullscreen.hlsli"

// 乱数生成関数　https://www.ronja-tutorials.com/post/024-white-noise/
float rand2dTo1d(float2 value, float2 dotDir = float2(12.9898, 78.233))
{
    float2 smallValue = sin(value);
    float random = dot(smallValue, dotDir);
    random = frac(sin(random) * 143758.5453);
    return random;
}

float rand(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

//パーリンノイズ
// https://zenn.dev/kento_o/articles/08ec03e29ed636
float perlinNoise(half2 st)
{
    half2 p = floor(st);
    half2 f = frac(st);
    half2 u = f * f * (3.0 - 2.0 * f);

    float v00 = rand(p + half2(0, 0));
    float v10 = rand(p + half2(1, 0));
    float v01 = rand(p + half2(0, 1));
    float v11 = rand(p + half2(1, 1));

    return lerp(lerp(dot(v00, f - half2(0, 0)), dot(v10, f - half2(1, 0)), u.x),
                            lerp(dot(v01, f - half2(0, 1)), dot(v11, f - half2(1, 1)), u.x),
                            u.y) + 0.5f;
}

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

struct Material
{
    float time;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<Material> gMaterial : register(b0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    float2 uv = input.texcoord;
    
    // ポスタライズ
    float posterrize1 = floor(frac(perlinNoise(gMaterial.time) * 10.0f) / (1.0f / 60.0f)) * (1.0f / 60.0f);
    float posterrize2 = floor(frac(perlinNoise(gMaterial.time) * 10.0f) / (1.0f / 60.0f)) * (1.0f / 60.0f);
    
    // uv.x方向のノイズ計算 -1 < noiseX , 1
    float noiseX = (2.0f * rand(posterrize1) - 0.5f) * 0.1f;
    
    // step(t,x)はxがtより大きい場合1を返す
    float frequency = step(rand(posterrize2), 0.5f);
    noiseX *= frequency;
    
    // uv.y方向のノイズ計算 -1 < noiseY , 1
    float noiseY = 2.0f * rand(posterrize1) - 0.5f;
    
    // グリッチの高さの補間値計算
    
    float glitchLine1 = step(uv.y - noiseY, rand(uv));
    float glitxhLine2 = step(uv.y + noiseY, noiseY);
    
    float glitch = saturate(glitchLine1 - glitxhLine2);

    uv.x = lerp(uv.x, uv.x + noiseX, glitch);
    
    output.color = gTexture.Sample(gSampler, uv);
    
    return output;
}