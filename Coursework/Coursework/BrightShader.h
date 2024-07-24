#pragma once

#include "BaseShader.h"
#include "BasicShaderParameters.h"

using namespace std;
using namespace DirectX;

class BrightShader : public BaseShader
{
public:
	BrightShader(ID3D11Device* device, HWND hwnd);
	~BrightShader();

	void setShaderParameters(BasicShaderParameters basicShaderParameters);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps) override;

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* clampSampleState;
};
