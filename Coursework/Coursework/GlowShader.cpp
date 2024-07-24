#include "GlowShader.h"

GlowShader::GlowShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"glow_vs.cso", L"glow_ps.cso");
}


GlowShader::~GlowShader()
{
	// Release the sampler texture state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void GlowShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerTextDesc;
	D3D11_SAMPLER_DESC samplerGlowTextDesc;

	D3D11_BUFFER_DESC glowColourBufferDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture sampler state description.
	samplerTextDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerTextDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerTextDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerTextDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerTextDesc.MipLODBias = 0.0f;
	samplerTextDesc.MaxAnisotropy = 1;
	samplerTextDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerTextDesc.BorderColor[0] = 0;
	samplerTextDesc.BorderColor[1] = 0;
	samplerTextDesc.BorderColor[2] = 0;
	samplerTextDesc.BorderColor[3] = 0;
	samplerTextDesc.MinLOD = 0;
	samplerTextDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerTextDesc, &sampleState);

	// Sampler for shadow map sampling.
	samplerGlowTextDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerGlowTextDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerGlowTextDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerGlowTextDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerGlowTextDesc.MipLODBias = 0.0f;
	samplerGlowTextDesc.MaxAnisotropy = 1;
	samplerGlowTextDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerGlowTextDesc.BorderColor[0] = 1.0f;
	samplerGlowTextDesc.BorderColor[1] = 1.0f;
	samplerGlowTextDesc.BorderColor[2] = 1.0f;
	samplerGlowTextDesc.BorderColor[3] = 1.0f;
	samplerGlowTextDesc.MinLOD = 0;
	samplerGlowTextDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerGlowTextDesc, &sampleStateGlow);

	// Setup Glow colour buffer
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	glowColourBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	glowColourBufferDesc.ByteWidth = sizeof(GlowColourBufferType);
	glowColourBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	glowColourBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	glowColourBufferDesc.MiscFlags = 0;
	glowColourBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&glowColourBufferDesc, NULL, &glowColourBuffer);
}


void GlowShader::setShaderParameters(BasicShaderParameters& basicShaderParameters, ID3D11ShaderResourceView* glowMapTexture, XMFLOAT4 glowColour)
{
	// Variables to use along the funcion
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to.
	MatrixBufferType* dataPtr = nullptr;
	result = basicShaderParameters.deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = XMMatrixTranspose(basicShaderParameters.worldMatrix);
	dataPtr->view = XMMatrixTranspose(basicShaderParameters.viewMatrix);
	dataPtr->projection = XMMatrixTranspose(basicShaderParameters.projectionMatrix);
	basicShaderParameters.deviceContext->Unmap(matrixBuffer, 0);
	basicShaderParameters.deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// Set shader texture and sample resources in the pixel shader.
	basicShaderParameters.deviceContext->PSSetShaderResources(0, 1, &(basicShaderParameters.texture));
	basicShaderParameters.deviceContext->PSSetShaderResources(1, 1, &glowMapTexture);

	basicShaderParameters.deviceContext->PSSetSamplers(0, 1, &sampleState);
	basicShaderParameters.deviceContext->PSSetSamplers(1, 1, &sampleStateGlow);

	// TO VS and PS
	GlowColourBufferType* glowColourPtr;
	basicShaderParameters.deviceContext->Map(glowColourBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	glowColourPtr = (GlowColourBufferType*)mappedResource.pData;
	glowColourPtr->glowColour = glowColour;

	basicShaderParameters.deviceContext->Unmap(glowColourBuffer, 0);
	basicShaderParameters.deviceContext->PSSetConstantBuffers(0, 1, &glowColourBuffer);
}
