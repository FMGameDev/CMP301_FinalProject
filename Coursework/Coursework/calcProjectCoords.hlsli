// Calculate the projected texture coordinates.
// View pos can be the light view position when you are using the depth map
// Or camera view pos if you are using the shadow texture (black/white)
float2 getProjectiveCoords(const float4 viewPosition)
{
    // Calculate the projected texture coordinates.
    float2 projTex = viewPosition.xy / viewPosition.w;
    projTex *= float2(0.5f, -0.5f); // divide by 2
    projTex += float2(0.5f, 0.5f); // add 0.5f
    return projTex;
}