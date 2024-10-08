// Depth vertex shader
// It is used for calculating the depth of a light using its matrices

//////////////////////
// CONSTANT BUFFERS //
//////////////////////

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix; // light view matrix
    matrix projectionMatrix; // projection matrix
};


//////////////
// TYPEDEFS //
//////////////

struct InputType
{
    float4 position : POSITION;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXCOORD0;
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

    // Store the position value in a second input value for depth value calculations.
    output.depthPosition = output.position;
	
    return output;
}