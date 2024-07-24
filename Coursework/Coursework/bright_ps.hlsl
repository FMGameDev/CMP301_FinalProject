
//////////////////////////////
// TEXTURES & SAMPLES STATES//
//////////////////////////////
// The render to texture of the regular scene is sent into the shader as colorTexture.
Texture2D sceneTexture : register(t0);

SamplerState textureSampler : register(s0);


//////////////////////
// CONSTANT BUFFERS //
//////////////////////

// The glowStrength variable in the GlowBuffer is for increasing the blurred glow map texture so that the glow has a stronger effect.
cbuffer GlowBuffer : register(b0) // per frame
{
    float4 glowColour;
};


//////////////
// TYPEDEFS //
//////////////

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float4 viewPos : TEXCOORD1;
};



float4 brightFilter(const float4 textureColour)
{
    /// Reference: https://learnopengl.com/Advanced-Lighting/Bloom
    
    // Luminance vector and equation from:
    const float3 luminanceVector = float3(0.2126, 0.7152, 0.0722); // weighting coefficients for luminance
    const float brigthFilter = 1.0f; // To determine if the brightness exceeds this threshold.

    // calculate the brightness by properly transforming it to grayscale
    // Brightness formula: Y = 0.2126*R + 0.7152*G + 0.0722*B:
    float brightness = dot(luminanceVector, textureColour.rgb);

    // As the pixel colours which should be glown excess 1.0 (previusly set on the colour shader
    // it is easily recognisable which glow
    if (brightness > brigthFilter) 
    {
        return textureColour;
    }
    else
    {
        return float4(0.0, 0.0, 0.0, 1.0); // black
    }
}


//////////////
//   MAIN   //
////////////// 

float4 main(InputType input) : SV_TARGET
{
    float4 textureColour = sceneTexture.Sample(textureSampler, input.tex);
    
    float4 colour = brightFilter(textureColour);
    
    return colour;
   
}