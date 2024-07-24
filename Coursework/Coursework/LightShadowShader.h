#pragma once

#include "DXF.h"
#include "LightsRegister.h"
#include "NormalCalcTypeBufferType.h"
#include "BasicShaderParameters.h"
#include "LightUtils.h"
#include "ShadowShaderParameters.h"

using namespace std;
using namespace DirectX;

class LightShadowShader : public BaseShader
{

private:
	// Type of lights

	struct DirectionalLight
	{
		// initialize complex variable to zero
		DirectionalLight() { ZeroMemory(this, sizeof(this)); }

		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT4 specular;

		// packed into 4D vector (direction, specularPower)
		XMFLOAT3 direction;
		float specularPower;

		XMFLOAT3 padding;
		float isOn;
	};

	struct PointLight
	{
		// initialize complex variable to zero
		PointLight() { ZeroMemory(this, sizeof(this)); }

		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT4 specular;

		// packed into 4D vector (position, specularPower)
		XMFLOAT3 position;
		float specularPower; // range = A point who distance from the light source is greater than the range is not lit

		// packed into 4D vector (attenuation, padding)
		XMFLOAT3 attenuation; // default values = (1, 0, 0) = (C, L, Q); C= Constant factor, L= Linear Factor, Q= Quadratic Factor   
		float isOn;
	};

	struct SpotLight
	{
		// initialize complex variable to zero
		SpotLight() { ZeroMemory(this, sizeof(this)); }

		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT4 specular;

		// packed into 4D vector (position, specularPower)
		XMFLOAT3 position;
		float specularPower; // range = A point who distance from the light source is greater than the range is not lit

		// packed into 4D vector (direction, spot exponent)
		XMFLOAT3 direction;
		float spotExponent;

		// packed into 4D vector (attenuation, padding)
		XMFLOAT3 attenuation; // default values = (1, 0, 0) = (C, L, Q); C= Constant factor, L= Linear Factor, Q= Quadratic Factor   
		float isOn;
	};

	
	// Type of buffers

	struct LightsBufferType
	{
		// initialize complex variable to zero
		LightsBufferType() { ZeroMemory(this, sizeof(this)); }

		DirectionalLight dLights[kDirectionalLightNum];
		PointLight pLights[kPointLightNum];
		SpotLight sLights[kSpotLightNum];
	};

	struct CameraBufferType
	{
		XMFLOAT3 camera_pos;
		float padding;
	};

	struct LightsMatrixBufferType
	{
		XMMATRIX dLightViews[kDirectionalLightNum];
		XMMATRIX dLightProjections[kDirectionalLightNum];

		XMMATRIX sLightViews[kSpotLightNum];
		XMMATRIX sLightProjections[kSpotLightNum];

		XMMATRIX pLightViews[kPointLightNum * kPointLightFacesNum];
		XMMATRIX pLightProjections[kPointLightNum * kPointLightFacesNum];
	};

public:
	LightShadowShader(ID3D11Device* device, HWND hwnd, bool inApplyDisplacementMapping = false);
	~LightShadowShader();

	void setShaderParameters(BasicShaderParameters& basicShaderParameters, ShadowShaderParameters& shadowShaderParameters,
		Lights& lights, Camera* camera, NormalCalcTypeBufferType displacementMappingBufferType);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps) override;

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* lightsMatrixBuffer;
	ID3D11SamplerState* sampleStateTexture; // sample texture
	ID3D11SamplerState* sampleStateShadow; // sample shadow
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* cameraBuffer;

	bool applyDisplacementMapping;
	// Elements for VS and PS
	ID3D11Buffer* normalCalcTypeBuffer;
};
