#pragma once

#include "BaseShader.h"
#include "BasicShaderParameters.h"

using namespace std;
using namespace DirectX;


class ColourShader : public BaseShader
{
private:
	struct ColourBufferType
	{
		XMFLOAT4 colour;
	};
public:
	ColourShader(ID3D11Device* device, HWND hwnd);
	~ColourShader();

	void setShaderParameters(BasicShaderParameters basicShaderParameters, XMFLOAT4 colour);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps) override;

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* colourBuffer;
};
