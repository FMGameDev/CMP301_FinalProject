#ifndef _LIGHTS_REGISTER_H
#define _LIGHTS_REGISTER_H
// Register of how many lights there are:
// If these values are modifed you need to:
// - In CPU: Add and initialise the lights objects if the number of lights are increased or delet lights objects if the number is decreased in App1 file
// More about variable syntax: https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-variable-syntax


static const int kDirectionalLightNum = 2;
static const int kSpotLightNum = 1;
static const int kPointLightNum = 1;

// The point light will have a cube behaviour
// for the rest of lights there will be only one "face"
static const int kPointLightFacesNum = 6;

#endif// _LIGHTS_REGISTER_H