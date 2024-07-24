// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct InputType
{
    float3 position : POSITION;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXCOORD0;
};

[domain("quad")]
OutputType main(ConstantOutputType input, float2 uvCoord : SV_DomainLocation, const OutputPatch<InputType, 4> patch)
{
    float3 vertexPosition;
    OutputType output;
 
    // Determine the position of the new vertex (Bilineal Interpolation)
	// Deal with y coords
    // Linearly interpolate(lerp) between 0 and 1
    float3 v1 = lerp(patch[0].position, patch[1].position, uvCoord.y); // if triangle_ccw then is uvwCoord.y
    // Linearly interpolate(lerp) between 3 and 2
    float3 v2 = lerp(patch[3].position, patch[2].position, uvCoord.y);
    // The final vertex position will be the lerp result between v1, v2 based on our x-coords
    vertexPosition = lerp(v1, v2, uvCoord.x);
     
    // Calculate the position of the new vertex against the world, view, and projection matrices.
    output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    // Store the position value in a second input value for depth value calculations.
    output.depthPosition = output.position;

    return output;
}
