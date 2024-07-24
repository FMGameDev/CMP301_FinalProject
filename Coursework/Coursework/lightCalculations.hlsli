// Light Utils constains functions for calculating the lighting with shadows

#include "calcProjectCoords.hlsli"
#include "LightsRegister.h"

//////////////
// TEXTURES //
//////////////
Texture2D shaderTexture : register(t0);

Texture2D dShadowTexture[kDirectionalLightNum] : register(t1); // the shadow textures are the black and white blurred image that contain soft shadows
Texture2D sShadowTexture[kSpotLightNum] : register(t3);
Texture2D pShadowTextures[kPointLightNum * kPointLightFacesNum] : register(t4);

///////////////////
// SAMPLE STATES //
///////////////////
SamplerState textureSampler : register(s0);
SamplerState shadowSampler : register(s1);

//////////////
// TYPEDEFS //
//////////////

struct DirectionalLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    
    float3 direction;
    float specularPower;
    
    float3 padding;
    float isOn;
};

struct PointLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    
    float3 position;
    float specularPower; // range;-> range = A point who distance from the light source is greater than the range is not lit
    
    float3 attenuation; // default values = (1, 0, 0) = (C, L, Q); C= Constant factor, L= Linear Factor, Q= Quadratic Factor   
    float isOn;
};

struct SpotLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    
    float3 position;
    float specularPower; // range;-> range = A point who distance from the light source is greater than the range is not lit
    
    float3 direction;
    float spotExponent;
    
    float3 attenuation; // default values = (1, 0, 0) = (C, L, Q); C= Constant factor, L= Linear Factor, Q= Quadratic Factor   
    float isOn;
};


//////////////////////
// CONSTANT BUFFERS //
//////////////////////

cbuffer LightBuffer : register(b0) // per frame
{
    DirectionalLight bDirLight[kDirectionalLightNum];
    PointLight bPointLight[kSpotLightNum];
    SpotLight bSpotLight[kPointLightNum];
};


//////////////
// FUNCTIONS //
//////////////

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calcDiffuse(float3 lightVector, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightVector));
    return diffuse * intensity;
}

// Calculate Specular Factor (blinn-phong)
float4 calcSpecular(float3 lightVector, float3 normal, float3 viewVector, float4 specularColour, float specularPower)
{
    float3 halfway = normalize(lightVector + viewVector);
    float specularIntensity = saturate(pow(max(dot(normal, halfway), 0.0), specularPower));
    return specularColour * specularIntensity;
}

// Calculate Attenuation
float calcAttenuation(float constantFactor, float linearFactor, float quadraticFactor, float distance)
{
    return (1.0f / (constantFactor + (linearFactor * distance) + (quadraticFactor * pow(distance, 2.0f))));
}


// Compute Directional Light
void computeDirectionalLight(DirectionalLight dLight, float3 normal, float3 viewVector, out float4 ambient, out float4 diffuse, out float4 specular)
{
    // initialize outputs first
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // the light vector aims opposite the direction the light rays travel
    float3 lightVector = -dLight.direction;
    
    diffuse = calcDiffuse(lightVector, normal, dLight.diffuse);
    specular = calcSpecular(lightVector, normal, viewVector, dLight.specular, dLight.specularPower);
    ambient = dLight.ambient;
}

// Compute Point Light
void computePointLight(PointLight pLight, float3 normal, float3 worldPosition, float3 viewVector, out float4 ambient, out float4 diffuse, out float4 specular)
{
    // initialize outputs first
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // the vector from the surface to the light position
    float3 lightVector = pLight.position - worldPosition;
    // the distance from the surface to the light
    float distance = length(lightVector);
    // normalise ligth vector to use it in the following calculations
    lightVector = normalize(lightVector);
    
    diffuse = calcDiffuse(lightVector, normal, pLight.diffuse);
    specular = calcSpecular(lightVector, normal, viewVector, pLight.specular, pLight.specularPower);
    ambient = pLight.ambient;
    
    // Attenuate
    float attenuation = calcAttenuation(pLight.attenuation.x, pLight.attenuation.y, pLight.attenuation.z, distance); // http://learnwebgl.brown37.net/09_lights/lights_attenuation.html  
    diffuse *= attenuation;
    specular *= attenuation;
}

// Compute SpotLight
void computeSpotLight(SpotLight sLight, float3 normal, float3 worldPosition, float3 viewVector, out float4 ambient, out float4 diffuse, out float4 specular)
{
    // initialize outputs first
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // the vector from the surface to the light position
    float3 lightVector = sLight.position - worldPosition;
    // the distance from the surface to the light
    float distance = length(lightVector);
    // normalise ligth vector to use it in the following calculations
    lightVector = normalize(lightVector);

    diffuse = calcDiffuse(lightVector, normal, sLight.diffuse);
    specular = calcSpecular(lightVector, normal, viewVector, sLight.specular, sLight.specularPower);
    ambient = sLight.ambient;
    
    // Apply spotlight factor
    float spotFactor = pow(max(dot(lightVector, -sLight.direction), 0.0f), sLight.spotExponent);
    ambient *= spotFactor; // Apply to abient is optional
    diffuse *= spotFactor;
    specular *= spotFactor;
    
    // Attenuate
    float attenuation = calcAttenuation(sLight.attenuation.x, sLight.attenuation.y, sLight.attenuation.z, distance); // http://learnwebgl.brown37.net/09_lights/lights_attenuation.html  
    diffuse *= attenuation;
    specular *= attenuation;
}


// Return the shadow value sampling the texture
float getShadowValue(const Texture2D shadowTexture, const SamplerState shadowSampler, const float2 uv)
{
    float shadowValue = shadowTexture.Sample(shadowSampler, uv).r;
       
    return shadowValue;
}

// Calculate the lighting of that pixel
float4 calcLighting(float3 normal, float4 viewPos, float3 camViewVector, float3 worldPosition, float4 dLightViewPos[kDirectionalLightNum], float4 pLightViewPos[kPointLightNum * kPointLightFacesNum], float4 sLightViewPos[kSpotLightNum])
{
    // initialize the final components (sum of all the lights) => Finals values
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // components where we will save the ambient, diffuse and specular values result => Temporals values
    float4 A, D, S;
    
    // calculate the projective coords from the camera view pos for using it to get the shadow value from the shadow texture
    float2 viewPosProjected = getProjectiveCoords(viewPos);
    
    // Compute the directional, point and spot lights and add them to the final light values if they are turned on
    
    // Directional light
    [unroll]
    for (int d = 0; d < kDirectionalLightNum; d++)
    {
        if (bDirLight[d].isOn == 1)
        {
            computeDirectionalLight(bDirLight[d], normal, camViewVector, A, D, S);
            ambient += A;
        
            // get the shadow value-> between 0 to 1 which will determ the % of light for this pixel
            float dShadowValue = getShadowValue(dShadowTexture[d], shadowSampler, viewPosProjected);
            diffuse += (D * dShadowValue);
            specular += (S * dShadowValue);
        }
    }

    // Point Light
    [unroll]
    for (int p = 0; p < kPointLightNum; p++)
    {
        if (bPointLight[p].isOn == 1)
        {
            computePointLight(bPointLight[p], normal, worldPosition, camViewVector, A, D, S);
            ambient += A;
        
            [unroll]
            for (uint f = 0; f < kPointLightFacesNum; f++)
            {
                // get the shadow value-> between 0 to 1 which will determ the % of light for this pixel
                float pShadowValue = getShadowValue(pShadowTextures[f + (p * kPointLightFacesNum)], shadowSampler, viewPosProjected);
                diffuse += (D * pShadowValue);
                specular += (S * pShadowValue);
            }
        
        }
    }

    // Spotlight
    [unroll]
    for (int s = 0; s < kSpotLightNum; s++)
    {
        if (bSpotLight[s].isOn == 1)
        {
            computeSpotLight(bSpotLight[s], normal, worldPosition, camViewVector, A, D, S);
            ambient += A;

            // get the shadow value-> between 0 to 1 which will determ the % of light for this pixel
            float sShadowValue = getShadowValue(sShadowTexture[s], shadowSampler, viewPosProjected);
            diffuse += (D * sShadowValue);
            specular += (S * sShadowValue);
        }
    }
      
    // calculate the total light colour
    float4 lightColour = saturate(saturate(ambient) + saturate(diffuse) + saturate(specular));
    
    return lightColour;
}
