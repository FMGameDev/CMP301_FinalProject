//////////////////////
// CONSTANT BUFFERS //
//////////////////////

cbuffer BlurTypeBuffer : register(b0)
{
    float blurType;
    float3 padding;
};
// Total number of tex coord which will be taken into account to calc the colour of this pixel
// In this case is eight neighbours and the current tex coord
static const int kNumTexCoords = 9;
// the number of weights will be the half of the total number of tex coords plus one which is the current tex coord
static const int kNumWeight = 5; 


///////////////
// FUNCTIONS //
///////////////

float2 calcTexelSize(Texture2D texture2d)
{
    // Get width and height texture dimension
    uint textureWidth = 0;
    uint textureHeight = 0;
    texture2d.GetDimensions(textureWidth, textureHeight);
    
    // calculate texel
    float2 texelSize = float2(1.0f / textureWidth, 1.0f / textureHeight);
    
    return texelSize;
}

void calcNeighboursTexCoords(const Texture2D texture2d, const SamplerState samplerState, const float2 tex_coord, out float2 texCoordNeighbours[kNumTexCoords])
{     
    // Get the texel size
    float2 texelSize = calcTexelSize(texture2d);
    
    // Depending the blur type (horizontal or vertical)
    // we will annul the opposite variable (set to 0)
    // therefore no effect will take place when finding the neighbour calculations
    if (blurType == 1.0f) // horizontal blur thus annul vertical
        texelSize.y = 0.0f;
    else if (blurType == 2.0f) // vertical blur thus annul horizontal
        texelSize.x = 0.0f;
    
    // Find the neighbours coords using the texel size modified by the type of blur   
    float neighbourPos = -kNumWeight+1; // how many texel is from the current tex coord usign +/- coords
    for (int i = 0; i < kNumTexCoords; i++)
    {
        texCoordNeighbours[i] = tex_coord + float2(texelSize * neighbourPos);
        neighbourPos += 1.0f; // go to the next tex coord
    }
}

float4 applyBlur(const Texture2D texture2d, const SamplerState samplerState, const float2 texCoord)
{
    // Get the the neighbours tex coords
    float2 texCoordNeighbours[kNumTexCoords];
    calcNeighboursTexCoords(texture2d, samplerState, texCoord, texCoordNeighbours);
    
    // Gaussian Blur
    const float kWeight[kNumWeight] =
    {
        0.05f,
        0.09f,
        0.12f,
        0.15f,
        0.16f, // center pixel (the current)
    };
    
    // Apply blur
    // Add the vertical or horizontal pixels to the colour by the specific weight of each.  
    float4 colour = float4(0.0f, 0.0f, 0.0f, 0.0f); // initialise the colour to black as default
    
    // Go neighbour by neighbour adding their colour to the current pixel
    for (int coordId = 0, weightId = 0; coordId < kNumTexCoords; coordId++, weightId++)
    {    
        // control the weight id so it is never outside the array length
        // as the number of tex coords is double plus one bigger than the number of weights
        if (weightId >= kNumWeight)
        {
            weightId = 0;
        }
        // Add to the colour to the final
        colour += texture2d.Sample(samplerState, texCoordNeighbours[coordId]) * kWeight[weightId];
    }
    colour.a = 1.0f;
    
    return colour;
}