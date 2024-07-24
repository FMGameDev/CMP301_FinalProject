#include "displacementMappingHeight.hlsli"

//////////////////////////////
// TEXTURES & SAMPLES STATES//
//////////////////////////////
Texture2D textureHeightMap : register(t0);
SamplerState samplerHeightMap : register(s0);


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
    float2 tex : TEXCOORD0; // Texture coord used for getting the height from the texture
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
    
    // apply displacement mapping
    input.position.y = getHeightFromTexture(textureHeightMap, samplerHeightMap, input.tex);

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Store the position value in a second input value for depth value calculations.
    output.depthPosition = output.position;
	
    return output;
}