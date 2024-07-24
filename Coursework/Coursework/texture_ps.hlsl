// Texture Pixel Shader
// Added function of blurring if requested

#include "blur.hlsli" // Add the 


//////////////////////////////
// TEXTURES & SAMPLES STATES//
//////////////////////////////
Texture2D texture0 : register(t0);
SamplerState samplerState0 : register(s0);


//////////////
// TYPEDEFS //
//////////////

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};


//////////////
//   MAIN   //
////////////// 

float4 main(InputType input) : SV_TARGET
{
    float4 colour;
    
    if (blurType == 0.0f) // do not apply blur
        colour = texture0.Sample(samplerState0, input.tex);
    else // apply blur
        colour = applyBlur(texture0, samplerState0, input.tex);
    
    return colour;
}
