// Colour shader.h
// Simple shader example.
#pragma once

#include "DXF.h"
#include "BasicShaderParameters.h"

using namespace std;
using namespace DirectX;


class DepthShader : public BaseShader
{

public:
	DepthShader(ID3D11Device* device, HWND hwnd, bool inApplyDisplacementMapping = false);
	~DepthShader();

	void setShaderParameters(BasicShaderParameters& basicShaderParameters);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps) override;

private:
	ID3D11Buffer* matrixBuffer;

	bool applyDisplacementMapping;
	ID3D11SamplerState* sampleState;
};
