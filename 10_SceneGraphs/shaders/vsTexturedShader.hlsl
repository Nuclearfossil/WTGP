#pragma shader_model 5.0

cbuffer ViewProjectionBuffer : register(b0)
{
    row_major matrix ViewProjection;
}

cbuffer LocalToWorldBuffer : register(b1)
{
    row_major matrix localToWorld;
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

    output.worldpos = mul(float4(input.position, 1.0f), localToWorld);
    output.position = mul(float4(input.position, 1.0f), mul(localToWorld, ViewProjection));
    output.color = input.color;
    output.normal = normalize(mul(input.normal, (float3x3) localToWorld));
    output.texCoord = input.texCoord;

    return output;
}
