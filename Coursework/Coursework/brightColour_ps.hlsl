// Colour Pixel Shader

//////////////
// TYPEDEFS //
//////////////

// The glowStrength variable in the GlowBuffer is for increasing the blurred glow map texture so that the glow has a stronger effect.
cbuffer ColourBuffer : register(b0) // per frame
{
    float4 glowColour;
};

struct InputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
};


//////////////
//   MAIN   //
////////////// 

float4 main(InputType input) : SV_TARGET
{
    float4 colour = glowColour * 1.5f; // amplify the colour so it will be detected as bright colour in the bright filter
    
    return colour;
}
