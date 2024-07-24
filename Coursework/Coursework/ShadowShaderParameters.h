#pragma once

#include "LightsRegister.h"
#include "DXF.h"

using namespace std;
using namespace DirectX;

struct ShadowShaderParameters
{
	ID3D11ShaderResourceView* dLightShadowTextures[kDirectionalLightNum];
	ID3D11ShaderResourceView* pLightShadowTextures[kPointLightNum * kPointLightFacesNum];
	ID3D11ShaderResourceView* sLightShadowTextures[kSpotLightNum];
	float shadowNear;
	float shadowFar;
};