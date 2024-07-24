// Texture Pixel Shader
// Added function of blurring if requested

//////////////////////////////
// TEXTURES & SAMPLES STATES//
//////////////////////////////
Texture2D sceneTexture : register(t0);
Texture2D brightTexture : register(t1); // post process texture (glow)

SamplerState clampSamplerState : register(s0);


//////////////
// TYPEDEFS //
//////////////

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

// Exposure tone mapping algorithm
// https://learnopengl.com/Advanced-Lighting/HDR
float4 ApplyToneMapping(const float4 hdrColour)
{
    const float gamma = 2.2;
    // for exposure the standard value is 1.0f.
    // Highest exposure make darker areas show more details
    // Lowest values will remove the dark region details.
    const float exposure = 1.0f;
    
    float4 ldrColour =float4(1.0f, 1.0f, 1.0f, 1.0f) - exp(-hdrColour.rgba * exposure);
    
    return ldrColour;
}


//////////////
//   MAIN   //
////////////// 

float4 main(InputType input) : SV_TARGET
{
    float4 brightColour = brightTexture.Sample(clampSamplerState, input.tex); // glow texture
    float4 sceneColour = sceneTexture.Sample(clampSamplerState, input.tex); 
    
    // the final colour will be a combination of both textures
    float4 colour = ApplyToneMapping(brightColour.rgba + sceneColour.rgba);
    
    return colour;
}
