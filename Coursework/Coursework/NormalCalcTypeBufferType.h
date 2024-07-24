#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

struct NormalCalcTypeBufferType
{
	NormalCalcTypeBufferType() :
		calcNormalOnPS(0.0f), calcAvgNormal(0.0f), renderNormalColour(0.0f),realWorldDimension(XMFLOAT2(0.0f, 0.0f)),
		padding1(0.0f), padding2(XMFLOAT2(0.0f, 0.0f)) {}

	NormalCalcTypeBufferType(float inCalcNormalOnPS, float inCalcAvgNormal, float inRenderNormalColour, XMFLOAT2 inRealWordDimension) :
		calcNormalOnPS(inCalcNormalOnPS), calcAvgNormal(inCalcAvgNormal), renderNormalColour(inRenderNormalColour), realWorldDimension(inRealWordDimension),
		padding1(0.0f), padding2(XMFLOAT2(0.0f, 0.0f)) {}

	float calcNormalOnPS;           // If Apply Displacement Mapping == 1 => 0 -> calculate normal on vertex shader, 1 -> calculate normal on pixel shader
	float calcAvgNormal;			// 0 -> calculate basic normal, 1 -> calculate average normal of the four normals of a point
	float renderNormalColour;       // 0 -> render light and texture colour, 1 -> render normal colour (red, green, blue -> x, y, z)
	float padding1;
	XMFLOAT2 realWorldDimension;		 // mesh dimension
	XMFLOAT2 padding2;
};