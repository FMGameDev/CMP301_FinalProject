// Basic shader for rendering coloured geometry (it is used for glowing)

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct InputType
{
    float4 position : POSITION;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};

OutputType main(InputType input)
{
    OutputType output;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.normal = input.normal;

    return output;
}