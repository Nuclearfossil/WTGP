cbuffer ViewProjectionBuffer : register(b0)
{
    row_major matrix ViewProjection;
}

cbuffer LocalToWorldBuffer : register(b1)
{
    row_major matrix localToWorld;
}

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
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float3 worldpos : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output = (VS_Output) 0;

    output.worldpos = mul(float4(input.position, 1.0f), localToWorld);
    output.position = mul(float4(input.position, 1.0f), mul(localToWorld, ViewProjection));
    output.color = input.color;
    output.normal = normalize(mul(input.normal, (float3x3) localToWorld));

    return output;
}

static const float4 minColor = (0.0, 0.0, 0.0, 0.0);
static const float4 maxColor = (1.0, 1.0, 1.0, 1.0);

// Simple Ambient + diffuse lighting model
float4 ps_main(VS_Output input) : SV_TARGET
{
    float4 ambient = input.color * .1;

    float3 lightDir = normalize(position - input.worldpos);
    float intensity = saturate(dot(input.normal, lightDir)); // this is the 'intensity' of the light
    float4 diffuse = input.color * intensity;

    return clamp(diffuse + ambient, minColor, maxColor);
}