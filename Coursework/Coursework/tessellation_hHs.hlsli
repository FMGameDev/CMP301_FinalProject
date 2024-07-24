cbuffer TessellationFactorBuffer : register(b0)
{
    float2 dMinMax; // dMinMax.x is dMin, dMinMax.y is dMax
    float2 lvlOfDetail; // lvlOfDetail.x is min lvl of detail and lvlOfDetail.y is the maximum
};

cbuffer CameraBuffer : register(b1)
{
    float3 cameraPos;
    float padding2;
};

cbuffer MatrixBuffer : register(b2)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};


// Find the center of patch in woold space
float CalculateDistanceFromCamera(float3 pos1, float3 pos2, float3 pos3, float3 pos4)
{
    float3 centerL, centerW;
    float3 d;
    
    // find the center of patch
    centerL = 0.25f * (pos1 + pos2 + pos3 + pos4);
    // calculate the center of patch against the world
    centerW = mul(float4(centerL, 1.0f), worldMatrix).xyz;
     
    // calculate the distance from the camera pos to the center of the quad
    d = distance(centerW, cameraPos);
    
    return d;
}

// Calculate tessellation Factor based on distance from eye camera
// The tessellation is 0 if d >= d1 and 64 if d <= d0.
// [d0, d1] defines the range we tessellate in.
float CalculateTessFactor(float3 d) // d is distance from camera
{
    const float dMin = dMinMax.x;
    const float dMax = dMinMax.y;
    const float loDMin = lvlOfDetail.x;
    const float loDMax = lvlOfDetail.y;
    
    // calculate the tessellation
    // formula: a+t*(b?a) -> lerp (interpolation t range [0,1])
    float t = saturate((dMax - d) / (dMax - dMin));
    float tess = loDMin + (t * (loDMax - loDMin));

    return tess;
}