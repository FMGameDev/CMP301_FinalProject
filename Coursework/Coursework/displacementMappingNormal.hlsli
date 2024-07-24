// Displacement Mapping Normal
// It contains the neccessary functions for calculating the normal using the height map texture
// It can be used in the vertex and pixel shader, depending where calculate the normal

#include "displacementMappingHeight.hlsli"

// calculate the texel and world cell space depending if it is in vertex or pixel shader
void calculateTexelAndWorldCellSpace(out float2 texelCellSpace, out float2 worldCellSpace, const Texture2D textureHeightMap, const float calcNormalOnPS, const float2 realWorldDimension)
{
    uint width = 0, height = 0; // Final width and height we want to simulate/transform
    
    if (calcNormalOnPS == 0) // -> calculate Normal on Vertex Shader
    {
        width = realWorldDimension.x;
        height = realWorldDimension.y;
    }
    else // -> calculate Normal on Pixel Shader
    {
        textureHeightMap.GetDimensions(width, height);
    }
    
    // Calculate world cell space //
    // it is the space between cells in world space
	// In Pixel Shader => it is [the real world dimension -> number of vertices (mesh dimension)] / [the resolution of the texture -> number of pixeles] 
    // In Vertex Shader => it is [the real world dimension -> number of vertices (mesh dimension)] / [the real world dimension -> number of vertices (mesh dimension)] thus world cell would value 1.0f in x and y
    worldCellSpace.x = realWorldDimension.x / width; // (worldCellSpaceU)
    worldCellSpace.y = realWorldDimension.y / height; // (worldCellSpaceV)
    
    // Calculate texel cell space //
    // It is the texture coord space which is from 0 to 1 between 
    // In Pixel Shader => It is 1 / [the resolution of the texture (number of pixels the texture has)]
    // In vertex Shader => IT is 1 / [the number of vertices (mesh dimension)]
    texelCellSpace.x = 1.0f / (float) width; // pixels in x (texelCellSpaceU)
    texelCellSpace.y = 1.0f / (float) height; // pixels in y (texelCellSpaceV)
}

// Calculate and return the normal using the height map texture
// float4 position must be the vertex position before applying any transformation 
float3 getNormalFromHeightMapTex(const Texture2D textureHeightMap, const SamplerState samplerHeightMap, const float2 uv, const float4 position, const float calcNormalOnPS, const float2 realWorldDimension, const float calcAvgNormal)
{
    
    // Calculate world and texel cells
    float2 worldCellSpace = float2(0.0f, 0.0f);
    float2 texelCellSpace = float2(0.0f, 0.0f);
 
    calculateTexelAndWorldCellSpace(texelCellSpace, worldCellSpace, textureHeightMap, calcNormalOnPS, realWorldDimension);
    
	//
	// Estimate normal and tangent using central differences
	//
	 
	// Text coords (UVs)
	// North, south, west and east texture coords are signed in the texture space
    float2 northTex = uv + float2(0.0f, +texelCellSpace.y); // up positive
    float2 southTex = uv + float2(0.0f, -texelCellSpace.y); // down negative
    float2 westTex = uv + float2(-texelCellSpace.x, 0.0f); // left negative
    float2 eastTex = uv + float2(+texelCellSpace.x, 0.0f); // right positive
	
	// Calculate points from centre the point to the North, South, West and East//
	// North (forward to the screen)
    float3 northPoint = float3(position.x, getHeightFromTexture(textureHeightMap, samplerHeightMap, northTex), position.z + worldCellSpace.y); // Stepping to the north is set positive
	// South (backward to us)
    float3 southPoint = float3(position.x, getHeightFromTexture(textureHeightMap, samplerHeightMap, southTex), position.z - worldCellSpace.y); // Stepping to the south is set negative
	// West (left)
    float3 westPoint = float3(position.x - worldCellSpace.x, getHeightFromTexture(textureHeightMap, samplerHeightMap, westTex), position.z); // Stepping to the weast is set negative
	// East (right)
    float3 eastPoint = float3(position.x + worldCellSpace.x, getHeightFromTexture(textureHeightMap, samplerHeightMap, eastTex), position.z); // Stepping to the east is set positive
	 
    if (calcAvgNormal == 0) // no calculate avg normal
    {
		// calculate tangents and normal
        float3 tangentEW = normalize(eastPoint - westPoint); // == normalize(float3(2.0f * worldCellSpace, rightY - leftY, 0.0f));
        float3 bitanSN = normalize(southPoint - northPoint); // == normalize(float3(0.0f, bottomY - topY, -2.0f * worldCellSpace));

        float3 normal = cross(tangentEW, bitanSN);
        return normal;
    }
    else // calculate avg normal
    {
		// Calculate East-North normal
		// Calculate E,N Vectors (final point - initial point)
        float3 E = normalize(eastPoint - position.xyz);
        float3 N = normalize(northPoint - position.xyz);
        float3 normalNE = cross(N, E);
	
		// Calculate South-East normal
		// Calculate S Vector (final point - initial point)
        float3 S = normalize(southPoint - position.xyz);
        float3 normalES = cross(E, S);
	
		// Calculate West-South normal
		// Calculate W Vector (final point - initial point)
        float3 W = normalize(westPoint - position.xyz);
        float3 normalSW = cross(S, W);
		
		// Calculate North-West normal
        float3 normalWN = cross(W, N);
	
		// Calculate the average normal which will be the final normal
        float3 avgNormal = (normalNE + normalES + normalSW + normalWN) / 4.0f;
        return avgNormal;
    }
}