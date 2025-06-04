#include "Fullscreen.hlsli"

static const uint kNumVertex = 3;
static const float4 kPositions[kNumVertex] =
{
    { -1.0f, 1.0f, 0.0f, 1.0f }, // ����
    { 3.0f, 1.0f, 0.0f, 1.0f },  // �E��
    { -1.0f, -3.0f, 0.0f, 1.0f } // ����
};

static const float2 kTexcoords[kNumVertex] =
{
    { 0.0f, 0.0f }, // ����
    { 2.0f, 0.0f }, // �E��
    { 0.0f, 2.0f }, // ����
};

VertexShaderOutput main(uint vertexId : SV_VertexID)
{
    VertexShaderOutput output;
    output.position = kPositions[vertexId];
    output.texcoord = kTexcoords[vertexId];
    return output;
}