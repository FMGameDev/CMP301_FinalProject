// Depth Pixel Shader
// It return the depth colour for a pixel for a single Light

//////////////
// TYPEDEFS //
//////////////

struct InputType
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXCOORD0;
};


//////////////
//   MAIN   //
////////////// 

float4 main(InputType input) : SV_TARGET
{
    float depthValue;
    float4 colour;
    
	// Get the depth value of the pixel by dividing the Z pixel depth by the homogeneous W coordinate.
    depthValue = input.depthPosition.z / input.depthPosition.w;
    
    colour =  float4(depthValue, depthValue, depthValue, 1.0f);
    
    return colour;
}