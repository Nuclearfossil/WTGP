#pragma shader_model 5.0

cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix ModelViewProjection;
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

VS_Output vs_main(VS_Input input)
{
    VS_Output output = (VS_Output) 0;

    output.worldpos = mul(float4(input.position, 1.0f), World);
    output.position = mul(float4(input.position, 1.0f), ModelViewProjection);
    output.color = input.color;
    output.normal = normalize(mul(input.normal, (float3x3) World));
    output.texCoord = input.texCoord;

    return output;
}
