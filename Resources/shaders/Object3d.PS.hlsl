#include "object3d.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
    float shininess;
    float3 specularColor;
};

struct DirectionLight
{
    float4 color;
    float3 direction;
    float intensity;
};

struct PointLight
{
    float4 color; // ���C�g�̐F
    float3 position; // ���C�g�̐F
    float intensity; // �P�x
    float radius; // ���C�g�̓͂��ő勗��
    float decay; // ������
};

struct SpotLight
{
    float4 color; // ���C�g�̐F
    float3 position; // ���C�g�̈ʒu
    float intensity; // �P�x
    float3 direction; // ���C�g�̕���
    float distance; // ���C�g�̓͂��ő勗��
    float decay; // ������
    float cosAngle; // �X�|�b�g���C�g�̗]��
    float cosFalloffStart;
};

ConstantBuffer<Material> gMaterial : register(b0);

ConstantBuffer<DirectionLight> gDirectionalLight : register(b1);

ConstantBuffer<PointLight> gPointLight : register(b2);

ConstantBuffer<SpotLight> gSpotLight : register(b3);


Texture2D<float4> gTexture : register(t0);

SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

float3 CalculationDirectionalLight(VertexShaderOutput input)
{
    float3 resultColor;
    
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    float3 toEye = normalize(input.cameraPosition - input.worldPosition);
    
    
         // Phong Reflection��������������������������������������������������������������������������������������������������������������
        
        //float3 reflectLight = reflect(gDirectionalLight.direction, normalize(input.normal));
        
        //float RdotE = dot(reflectLight, toEye);
        
        //float specularPow = pow(saturate(RdotE), gMaterial.shininess);
        
        // Phong Reflection����������������������������������������������������������������������������������������������������������������
        
        
        
        // BlinnPhong Reflection��������������������������������������������������������������������������������������������������������������

    float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
        
    float NDotH = dot(normalize(input.normal), halfVector);
        
    float specularPow = pow(saturate(NDotH), gMaterial.shininess);
        
        // BlinnPhong Reflection����������������������������������������������������������������������������������������������������������������
        
    float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        
    float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        
        //output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;  
        
        
        // �g�U����
    float3 directionalLight_Diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        
        // ���ʔ���
    float3 directionalLight_Specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * gMaterial.specularColor;
    
    resultColor = directionalLight_Diffuse + directionalLight_Specular;
    
    return resultColor;
}

float3 CalculationPointLight(VertexShaderOutput input)
{
    float3 resultColor;
    
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    float3 toEye = normalize(input.cameraPosition - input.worldPosition);
  
        
    float distance = length(gPointLight.position - input.worldPosition);
        
    float factor = pow(saturate(-distance / gPointLight.radius + 1.0), gPointLight.decay);
        
    float3 pointLightDirection = normalize(input.worldPosition - gPointLight.position);
        
    float3 pointLight_HalfVector = normalize(-pointLightDirection + toEye);
        
    float pointLight_NDotH = dot(normalize(input.normal), pointLight_HalfVector);
        
    float pointLight_SpecularPow = pow(saturate(pointLight_NDotH), gMaterial.shininess);
        
    float pointLight_NDotL = dot(normalize(input.normal), -pointLightDirection);
        
    float pointLight_Cos = pow(pointLight_NDotL * 0.5 + 0.5f, 2.0f);
       
        // �g�U����
    float3 pointLight_Diffuse = gMaterial.color.rgb * textureColor.rgb * gPointLight.color.rgb * pointLight_Cos * gPointLight.intensity * factor;
        
        // ���ʔ���
    float3 pointLight_Specular = gPointLight.color.rgb * gPointLight.intensity * pointLight_SpecularPow * factor * gMaterial.specularColor;
    
    resultColor = pointLight_Diffuse + pointLight_Specular;
    
    return resultColor;
}

float3 CalculationSpottLight(VertexShaderOutput input)
{
    float3 resultColor;
    
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    float3 toEye = normalize(input.cameraPosition - input.worldPosition);
   
        
    float spotLight_Distance = length(gSpotLight.position - input.worldPosition); // �|�C���g���C�g�ւ̋���
        
    float attenuationFactor = pow(saturate(-spotLight_Distance / gSpotLight.distance + 1.0), gSpotLight.decay);
        
    float3 spotLightDirectionOnSurface = normalize(input.worldPosition - gSpotLight.position);
        
    float cosAngle = dot(spotLightDirectionOnSurface, gSpotLight.direction);
        
    float falloffFactor = saturate((cosAngle - gSpotLight.cosAngle) / (gSpotLight.cosFalloffStart - gSpotLight.cosAngle));
        
    float3 spotLight_HalfVector = normalize(-gSpotLight.direction + toEye);
        
    float spotLight_NDotH = dot(normalize(input.normal), spotLight_HalfVector);
        
    float spotLight_SpecularPow = pow(saturate(spotLight_NDotH), gMaterial.shininess);
        
    float spotLight_NdotL = dot(normalize(input.normal), -gSpotLight.direction);
        
    float spotLight_Cos = pow(spotLight_NdotL * 0.5f + 0.5f, 2.0f);
        
        // �g�U����
    float3 spotLight_Diffuse = gMaterial.color.rgb * textureColor.rgb * gSpotLight.color.rgb * gSpotLight.intensity * falloffFactor * attenuationFactor * spotLight_Cos;
        
        // ���ʔ���
    float3 spotLight_Specular = gSpotLight.color.rgb * gSpotLight.intensity * spotLight_SpecularPow * falloffFactor * attenuationFactor * gMaterial.specularColor;
    
    resultColor = spotLight_Diffuse + spotLight_Specular;
    
    return resultColor;
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float4(input.texcoord,0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    if (gMaterial.enableLighting != 0)
    {
        float3 toEye = normalize(input.cameraPosition - input.worldPosition);
        
        float3 directionalLightColor = CalculationDirectionalLight(input);
        
        float3 pointLightColor = CalculationPointLight(input);
        
        float3 spotLightColor = CalculationSpottLight(input);
       
        output.color.rgb = directionalLightColor + pointLightColor + spotLightColor;
        
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    
    
    return output;
}

