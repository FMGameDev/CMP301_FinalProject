// Light pixel shader with displacement mapping
// Calculate lights with shadows

#include "lightCalculations.hlsli"
#include "displacementMappingNormal.hlsli"
#include "lightsRegister.h"

//////////////////////
// CONSTANT BUFFERS //
//////////////////////
cbuffer NormalCalculationBuffer : register(b1) //-> LightBuffer is b0
{
    float calcNormalOnPS; // If Apply Displacement Mapping == 1 => 0 -> calculate normal on vertex shader, 1 -> calculate normal on pixel shader
    float calcAvgNormal; // 0 -> calculate basic normal, 1 -> calculate average normal of the four normals of a point
    float renderNormalColour; // 0 -> render light and texture colour, 1 -> render normal colour (red, green, blue -> x, y, z) 
    float padding1;
    float2 realWorldDimension; // mesh dimension
    float2 padding2;
};


//////////////
// TYPEDEFS //
//////////////

struct InputType // => Output type of Vertex Shader
{
    // basic output
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    // for displacement mapping
    float4 norm_position : POSITION; // position for normal calculation in ps. This position has not been transformed.
    
    // for light
    float3 worldPosition : POSITION1;
    float3 camViewVector : POSITION2;
 
    // for shadow
    float4 viewPos : TEXCOORD1;
    float4 dLightViewPos[kDirectionalLightNum] : TEXCOORD2;
    float4 sLightViewPos[kSpotLightNum] : TEXCOORD4;
    float4 pLightViewPos[kPointLightNum * kPointLightFacesNum] : TEXCOORD5;
};


//////////////
// FUNCTIONS //
//////////////

float4 main(InputType input) : SV_TARGET
{
    // Calculate normals on Pixel Shader if it is required
    // the shader texture will be the texture to use for displacemment mapping too in this case.
    if (calcNormalOnPS == 1.0f)
    {
        input.normal = getNormalFromHeightMapTex(shaderTexture, textureSampler, input.tex, input.norm_position, calcNormalOnPS, realWorldDimension, calcAvgNormal);
    }

	// Render colour normals
    if (renderNormalColour == 1.0f)
    {
        float4 normalColour = float4(input.normal.r, input.normal.g, input.normal.b, 1.0f); //for printing normals	
        return normalColour;
    }
	// Render lights and texture
    else
    {
        // sample the texture colour
        float4 textureColour = shaderTexture.Sample(textureSampler, input.tex);
    
        // calculate light colour
        float4 lightColour = calcLighting(input.normal, input.viewPos, input.camViewVector, input.worldPosition,
                                input.dLightViewPos, input.pLightViewPos, input.sLightViewPos);
    
        return saturate(lightColour * textureColour);
    }
}