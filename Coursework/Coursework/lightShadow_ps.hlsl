// Light pixel shader without displacement mapping
// Calculate lights with shadows

#include "lightCalculations.hlsli"

//////////////
// TYPEDEFS //
//////////////

struct InputType // => Output type of Vertex Shader
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    
    // for light
    float3 worldPosition : POSITION0;
    float3 camViewVector : POSITION1;
 
    // for shadow
    float4 viewPos : TEXCOORD1;
    float4 dLightViewPos[kDirectionalLightNum] : TEXCOORD2;
    float4 sLightViewPos[kSpotLightNum] : TEXCOORD4;
    float4 pLightViewPos[kPointLightNum * kPointLightFacesNum] : TEXCOORD5;
};


//////////////
//   MAIN   //
////////////// 

float4 main(InputType input) : SV_TARGET
{  
    // sample the texture colour
    float4 textureColour = shaderTexture.Sample(textureSampler, input.tex);
    
    // calculate light colour
    float4 lightColour = calcLighting(input.normal, input.viewPos, input.camViewVector, input.worldPosition,
                                input.dLightViewPos, input.pLightViewPos, input.sLightViewPos);
    
    return saturate(lightColour * textureColour);
}