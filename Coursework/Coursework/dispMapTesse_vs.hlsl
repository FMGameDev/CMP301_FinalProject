#include "displacementmappingheight_vsps.hlsli"

//////////////////////////////
// TEXTURES & SAMPLES STATES//
//////////////////////////////
Texture2D textureHeightMap : register(t0);
SamplerState samplerHeightMap : register(s0);


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
    float4 position : POSITION;
};


//////////////
//   MAIN   //
////////////// 

OutputType main(InputType input)
{
    OutputType output;

    // apply displacement mapping
    input.position.y = getHeightFromTexture(textureHeightMap, samplerHeightMap, input.tex);

    // Pass the vertex position into the hull shader.
    output.position = input.position;   
	
    return output;
}