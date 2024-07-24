#ifndef _LIGHT_UTILS_H
#define _LIGHT_UTILS_H

#pragma once
#include "DXF.h" 	// include dxframework
#include "LightsRegister.h"

enum class LightType
{
	kDirectional = 0,
	kSpotLight,
	kPointLight,
};

struct Lights
{
	Light* dLights[kDirectionalLightNum];	// for direcctional lights
	Light* pLights[kPointLightNum];			// for point lights
	Light* sLights[kSpotLightNum];			// for spotlights
};

#endif// _LIGHT_UTILS_H