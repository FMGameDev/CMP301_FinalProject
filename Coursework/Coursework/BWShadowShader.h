#pragma once

#include "DXF.h"
#include "BasicShaderParameters.h"

using namespace std;
using namespace DirectX;

class BWShadowShader : public BaseShader
{

private:

	// Type of buffers
	struct LightStateBufferType
	{
		// initialize complex variable to zero
		LightStateBufferType() { ZeroMemory(this, sizeof(this)); }

		float lightIsOn;
		XMFLOAT3 padding;
	};

	// New matrix buffer type for VS
	// The one on the base shader is not used this time
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;

		XMMATRIX lightView;
		XMMATRIX lightProjection;
	};

public:
	BWShadowShader(ID3D11Device* device,  HWND hwnd, bool inApplyDisplacementMapping = false);
	~BWShadowShader();

	void setShaderParameters(BasicShaderParameters& basicShaderParameters, ID3D11ShaderResourceView* depthMapTexture, const XMMATRIX& lightViewMatrix, const XMMATRIX& lightProjectionMatrix, bool lightIsOn);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleStateShadow; // sample shadow
	ID3D11Buffer* lightStateBuffer;

	bool applyDisplacementMapping;
	ID3D11SamplerState* sampleState; // sample state for texture height map (wrap sample)
};
