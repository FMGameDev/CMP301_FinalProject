#pragma once

#include "BaseShader.h"
#include "BasicShaderParameters.h"

using namespace std;
using namespace DirectX;

enum class BlurType
{
	None = 0, // Do not apply blur
	Horizontal, // Apply horizontal blur
	Vertical // Apply vertical blur
};

class TextureShader : public BaseShader
{
private:
	struct BlurTypeBufferType
	{
		float blurType;
		XMFLOAT3 padding;
	};
public:
	TextureShader(ID3D11Device* device, HWND hwnd);
	~TextureShader();

	void setShaderParameters(BasicShaderParameters basicShaderParameters, BlurType blurType = BlurType::None);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps) override;

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* clampSampleState;
	ID3D11SamplerState* wrapSampleState;
	ID3D11Buffer* blurTypeBuffer;
};

