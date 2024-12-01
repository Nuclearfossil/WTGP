#pragma shader_model 5.0

// Define texture and sampler
Texture2D diffuseTexture : register(t0);
SamplerState samplerState : register(s0);

cbuffer LightBuffer : register(b0)
{
    float3 position;
    float4 color;
}

struct VS_Input
{
    float3 position : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float3 worldpos : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

static const float4 minColor = float4(0.0, 0.0, 0.0, 0.0);
static const float4 maxColor = float4(1.0, 1.0, 1.0, 1.0);

// Simple Ambient + diffuse lighting model
float4 ps_main(VS_Output input) : SV_TARGET
{
    float4 ambient = input.color * .1;

    float3 lightDir = normalize(position - input.worldpos);
    float intensity = saturate(dot(input.normal, lightDir)); // this is the 'intensity' of the light
    float4 sampledTexture = diffuseTexture.Sample(samplerState, input.texCoord);
    float4 diffuse = sampledTexture * intensity;

    return clamp(diffuse + ambient, minColor, maxColor);
}