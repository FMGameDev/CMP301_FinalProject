// Light vertex shader with displacement mapping

#include "displacementMappingNormal.hlsli"
#include "LightsRegister.h"

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
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer LightsMatrixBuffer : register(b1)
{
    matrix dLightViewMatrix[kDirectionalLightNum];
    matrix dLightProjectionMatrix[kDirectionalLightNum];
    
    matrix sLightViewMatrix[kSpotLightNum];
    matrix sLightProjectionMatrix[kSpotLightNum];
    
    matrix pLightViewMatrix[kPointLightNum * kPointLightFacesNum];
    matrix pLightProjectionMatrix[kPointLightNum * kPointLightFacesNum];
};

cbuffer CameraBuffer : register(b2)
{
    float3 cameraPosition;
    float padding;
};

cbuffer NormalCalculationBuffer : register(b3)
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

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType // => Pixel Shader Input Type
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
    float4 pLightViewPos[kPointLightNum*kPointLightFacesNum] : TEXCOORD5;
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
    
    // Store the position of the vertice as viewed by the camera in a separate variable.
    //The viewPosition will be used to calculate the projection coordinates to project the soft shadows onto the scene.
    output.viewPos = output.position;

	// Store the texture coordinates for the pixel shader.
    output.tex = input.tex;

	// calculate normal using texture height map if it must be calculated on VS
    if (calcNormalOnPS == 0)
    {
        output.normal = getNormalFromHeightMapTex(textureHeightMap, samplerHeightMap, input.tex, input.position, calcNormalOnPS, realWorldDimension, calcAvgNormal);
    }
    else
    {
        output.norm_position = input.position;
    }

	// Calculate world position
    output.worldPosition = mul(input.position, worldMatrix);
	
	// Calculate view vector
    output.camViewVector = cameraPosition.xyz - output.worldPosition;
    output.camViewVector = normalize(output.camViewVector);
    
    // Calculate the position of the vertice as viewed by the directional light source.
    [unroll]
    for (int d = 0; d < kDirectionalLightNum; d++)
    {
        output.dLightViewPos[d] = mul(input.position, worldMatrix);
        output.dLightViewPos[d] = mul(output.dLightViewPos[d], dLightViewMatrix[d]);
        output.dLightViewPos[d] = mul(output.dLightViewPos[d], dLightProjectionMatrix[d]);
    }
    
    // Calculate the position of the vertice as viewed by the spot light source.
    [unroll]
    for (int s = 0; s < kSpotLightNum; s++)
    {
        output.sLightViewPos[s] = mul(input.position, worldMatrix);
        output.sLightViewPos[s] = mul(output.sLightViewPos[s], sLightViewMatrix[s]);
        output.sLightViewPos[s] = mul(output.sLightViewPos[s], sLightProjectionMatrix[s]);
    }
    
    // Calculate the position of the vertice as viewed by the point light source.
    [unroll]
    for (int p = 0; p < kPointLightNum; p++)
    {
        for (int f = 0; f < kPointLightFacesNum; f++)
        {
            output.pLightViewPos[f + (p * kPointLightFacesNum)] = mul(input.position, worldMatrix);
            output.pLightViewPos[f + (p * kPointLightFacesNum)] = mul(output.pLightViewPos[f + (p * kPointLightFacesNum)], pLightViewMatrix[f + (p * kPointLightFacesNum)]);
            output.pLightViewPos[f + (p * kPointLightFacesNum)] = mul(output.pLightViewPos[f + (p * kPointLightFacesNum)], pLightProjectionMatrix[f + (p * kPointLightFacesNum)]);
        }
    }

    return output;
}