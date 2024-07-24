// Black and white shadow pixel shader

#include "calcProjectCoords.hlsli"

//////////////////////////////
// TEXTURES & SAMPLES STATES//
//////////////////////////////
Texture2D depthMapTexture : register(t0); // depth map texture contain the scene depth buffer rendered from light perspective
SamplerState shadowSampler : register(s0);


//////////////////////
// CONSTANT BUFFERS //
//////////////////////

cbuffer LightStateBuffer : register(b0) // per frame
{
    float lightIsOn;
    float3 padding;
};


//////////////
// TYPEDEFS //
//////////////

struct InputType // => Output type of Vertex Shader
{
    float4 position : SV_POSITION;

    // for shadow
    float4 lightViewPos : TEXCOORD0;
};


///////////////
// FUNCTIONS //
///////////////

// Determine if the projected coordinates are in the 0 to 1 range.
// If so then this pixel is in the view of the light.
// uv = projected texture coords
bool hasDepthData(const float2 uv)
{
    if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
    {
        return false;
    }
    return true;
}

// If so then this pixel is in the view of the light.
// uv = projected texture coords
bool isInShadow(const Texture2D depthMapTexture, const SamplerState shadowSampler, const float2 uv, const float4 lightViewPosition, const float bias)
{
     // Sample the shadow map depth value from the depth texture using the sampler at the projected texture coordinate location.
    float depthValue = depthMapTexture.Sample(shadowSampler, uv).r;
	// Calculate the depth from the light.
    float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
    // Subtract the bias from the lightDepthValue.
    lightDepthValue -= bias;

	// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
    if (lightDepthValue < depthValue)
    {
        return false;
    }
    return true;
}

// return black or white colour depeding if it is in shadow or not respectively.
float4 getPixelColour(const float4 lightViewPos)
{
    // initialize the colour to return
    // by default the pixel is in shadow
    float4 colour = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Set the bias value for fixing the floating point precision issues.
    const float shadowMapBias = 0.009f;
    
    // check if the light is on
    if (lightIsOn == 1)
    {
        float2 texCoord = getProjectiveCoords(lightViewPos);     
        // Shadow test. Has depth map data and it is not in shadow
        // If the light is in front of the object then light the pixel, if not then shadow this pixel since an object (occluder) is casting a shadow on it.
        if (hasDepthData(texCoord) && !isInShadow(depthMapTexture, shadowSampler, texCoord, lightViewPos, shadowMapBias))
        {
            colour = float4(1.0f, 1.0f, 1.0f, 1.0f); // paint white
        }
        // If this is outside the area of shadow map range then draw things normally with regular lighting.
        else if (!hasDepthData(texCoord))
        {
            colour = float4(1.0f, 1.0f, 1.0f, 1.0f); // paint white
        }

    }

    return colour; // if it gets to this point it means that the pixel is in shadow
}


//////////////
//   MAIN   //
////////////// 

float4 main(InputType input) : SV_TARGET
{
    // get pixel colour (black or white) depending if it is in the show or not
    float4 colour = getPixelColour(input.lightViewPos);

    return colour;
}