// Tessellation Hull Shader
// Prepares control points for tessellation

#include "tessellation_hHs.hlsli"

struct InputType
{
    float3 position : POSITION;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct OutputType
{
    float3 position : POSITION;
};


ConstantOutputType PatchConstantFunction(InputPatch<InputType, 12> inputPatch, uint patchId : SV_PrimitiveID)
{
    ConstantOutputType output;
    
    // Calculate the distance from the center of the quad to the camera position
    // distance from the current quad (centre) to the camera
    float d = CalculateDistanceFromCamera(inputPatch[0].position, inputPatch[1].position, inputPatch[2].position, inputPatch[3].position);
    
    // distance from the left neighbour quad to the camera
    float dLeft = CalculateDistanceFromCamera(inputPatch[8].position, inputPatch[9].position, inputPatch[1].position, inputPatch[0].position);
    // distance from the top neighbour quad to the camera
    float dTop = CalculateDistanceFromCamera(inputPatch[6].position, inputPatch[0].position, inputPatch[3].position, inputPatch[7].position);
    // distance from the right neighbour quad to the camera
    float dRight = CalculateDistanceFromCamera(inputPatch[3].position, inputPatch[2].position, inputPatch[4].position, inputPatch[5].position);
    // distance from the bottom neighbour quad to the camera
    float dBottom = CalculateDistanceFromCamera(inputPatch[1].position, inputPatch[10].position, inputPatch[11].position, inputPatch[2].position);
    

    // calculate the tessellation factor for the center of the current quad, based on the distance
    float tessFactorCenter = CalculateTessFactor(d);
    
    // Set the tessellation factor for tessallating inside the quad.
    output.inside[0] = tessFactorCenter; // u-axis (columns)
    output.inside[1] = tessFactorCenter; // v-axis (rows)

    // Set the tessellation factors for the four edges of the quad.
    output.edges[0] = min(tessFactorCenter, CalculateTessFactor(dLeft)); // left edge
    output.edges[1] = min(tessFactorCenter, CalculateTessFactor(dTop)); // top edge
    output.edges[2] = min(tessFactorCenter, CalculateTessFactor(dRight)); // right edge
    output.edges[3] = min(tessFactorCenter, CalculateTessFactor(dBottom)); // bottom edge
   
    return output;
}


[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(12)]
[patchconstantfunc("PatchConstantFunction")]
[maxtessfactor(64.0f)]
OutputType main(InputPatch<InputType, 12> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    OutputType output;

    // Set the position for this control point as the output position.
    output.position = patch[pointId].position;
    
    return output;
}