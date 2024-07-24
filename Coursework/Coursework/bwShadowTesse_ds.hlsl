// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices

//////////////////////
// CONSTANT BUFFERS //
//////////////////////

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
};


//////////////
// TYPEDEFS //
//////////////

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
    
    // for shadow
    float4 lightViewPos : TEXCOORD0;
};


//////////////
//   MAIN   //
////////////// 

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
    
    // Calculate the position of the vertice as viewed by the directional light source.
    output.lightViewPos = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.lightViewPos = mul(output.lightViewPos, lightViewMatrix);
    output.lightViewPos = mul(output.lightViewPos, lightProjectionMatrix);

    return output;
}

