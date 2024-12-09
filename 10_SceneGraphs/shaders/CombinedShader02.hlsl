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
};

struct VS_Output
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output = (VS_Output)0;

    matrix modelViewProjection = mul(localToWorld, ViewProjection);
    output.position = mul(float4(input.position,1.0f), modelViewProjection);
	output.color = input.color;

	return output;
}

float4 ps_main(VS_Output input) : SV_TARGET
{
    return input.color;
}
