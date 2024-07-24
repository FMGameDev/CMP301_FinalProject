#pragma once

#include "BaseShader.h"
#include "BasicShaderParameters.h"

using namespace std;
using namespace DirectX;

class PostProcessingShader : public BaseShader
{
public:
	PostProcessingShader(ID3D11Device* device, HWND hwnd);
	~PostProcessingShader();

	void setShaderParameters(BasicShaderParameters basicShaderParameters, ID3D11ShaderResourceView* glowPostProcessing);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps) override;

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* clampSampleState;
};
