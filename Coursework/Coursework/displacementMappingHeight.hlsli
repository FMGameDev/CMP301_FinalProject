// Contain a function to get the height from the height map texture
float getHeightFromTexture(const Texture2D textureHeightMap, const SamplerState samplerHeightMap, const float2 uv, const float height = 40.0f) // Get Height using Displacement Mapping
{
    return textureHeightMap.SampleLevel(samplerHeightMap, uv, 0.0f).r * height; // standard height will be the red colour (from 0 to 1)
}

