//#include "Fullscreen.hlsli"

//struct Material
//{
//    int numSamples;
//    float2 center;
//    float blurWidth;
//    float3 diff;
//};
   
//Texture2D<float4> gTexture : register(t0);
//SamplerState gSamplerLinear : register(s0);
//ConstantBuffer<Material> gMaterial : register(b0);

//struct PixelShaderOutput
//{
//    float4 color : SV_TARGET0;
//};

//PixelShaderOutput main(VertexShaderOutput input)
//{ 
//    // `diff` を動きの方向と強さとして利用
//    // `diff` は前フレームと現在フレームの差分で、カメラや物体の動きを示すベクトル
//    float2 direction = gMaterial.diff.xz; // x, y の差分を移動方向に使用

//    // 色の初期化
//    float3 outputColor = float3(0.0f, 0.0f, 0.0f);

//    // 動きの方向に沿って複数のサンプルを取る
//    for (int sampleIndex = 0; sampleIndex < gMaterial.numSamples; ++sampleIndex)
//    {
//        // 現在のテクスチャ座標に、動きの方向に沿ってサンプル点を進める
//        float2 texcoord = input.texcoord + direction * gMaterial.blurWidth * float(sampleIndex);

//        // サンプルを取る
//        outputColor.rgb += gTexture.Sample(gSamplerLinear, texcoord).rgb;
//    }

//    // サンプルの平均化
//    outputColor.rgb *= rcp(gMaterial.numSamples);

//    // 出力色を設定
//    PixelShaderOutput output;
//    output.color.rgb = outputColor;
//    output.color.a = 1.0f; // 不透明度を1に設定

//    return output;
//}

#include "Fullscreen.hlsli"

struct Material
{
    int numSamples;
    float2 center; // 中心座標 (通常: float2(0.5, 0.5))
    float blurWidth; // ブラーの幅
    float3 diff; // 動きの方向
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSamplerLinear : register(s0);
ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    float2 uv = input.texcoord;

    // 中心からの距離でブラー強度を決定
    float dist = distance(uv, gMaterial.center);
    
    float innerRadius = 0.3f; // これより内側はブラーなし
    float outerRadius = 1.0f; // これより外は最大ブラー

    float vignette = saturate((dist - innerRadius) / (outerRadius - innerRadius));
    
    // motion blur の方向
    float2 direction = gMaterial.diff.xz;

    // motion blur 用ブラーサンプリング
    float3 blurredColor = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < gMaterial.numSamples; ++i)
    {
        float2 offset = direction * gMaterial.blurWidth * float(i);
        blurredColor += gTexture.Sample(gSamplerLinear, uv + offset).rgb;
    }
    blurredColor *= rcp(gMaterial.numSamples);

    // 元の画像（中心部に使う）
    float3 originalColor = gTexture.Sample(gSamplerLinear, uv).rgb;

    // ブレンド
    float3 finalColor = lerp(originalColor, blurredColor, vignette);

    PixelShaderOutput output;
    output.color = float4(finalColor, 1.0f);
    return output;
}