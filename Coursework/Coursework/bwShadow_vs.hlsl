// Black and white shadow pixel shader

//////////////////////
// CONSTANT BUFFERS //
//////////////////////

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
};


//////////////
// TYPEDEFS //
//////////////

struct InputType
{
    float4 position : POSITION;
};

struct OutputType // => Pixel Shader Input Type
{
    // vertex pos
    float4 position : SV_POSITION;

    // for shadow
    float4 lightViewPos : TEXCOORD0;
};


//////////////
//   MAIN   //
////////////// 

OutputType main(InputType input)
{
    OutputType output;
    
    // Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Calculate the position of the vertice as viewed by the directional light source.
    output.lightViewPos = mul(input.position, worldMatrix);
    output.lightViewPos = mul(output.lightViewPos, lightViewMatrix);
    output.lightViewPos = mul(output.lightViewPos, lightProjectionMatrix);

    return output;
}
