#pragma once

#include "DXF.h"
#include "BasicShaderParameters.h"


using namespace std;
using namespace DirectX;

class GlowShader : public BaseShader
{

private:
	struct GlowColourBufferType
	{
		XMFLOAT4 glowColour;
	};

public:
	GlowShader(ID3D11Device* device, HWND hwnd);
	~GlowShader();

	void setShaderParameters(BasicShaderParameters& basicShaderParameters, ID3D11ShaderResourceView* glowMapTexture, XMFLOAT4 glowColour);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps) override;

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleStateGlow;
	ID3D11Buffer* glowColourBuffer;
};
