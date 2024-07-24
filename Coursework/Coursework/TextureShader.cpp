#include "TextureShader.h"



TextureShader::TextureShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"texture_vs.cso", L"texture_ps.cso");
}


TextureShader::~TextureShader()
{
	// Release the sampler state.
	if (clampSampleState)
	{
		clampSampleState->Release();
		clampSampleState = 0;
	}

	if (wrapSampleState)
	{
		wrapSampleState->Release();
		wrapSampleState = 0;
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


void TextureShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC clampSamplerDesc;
	D3D11_SAMPLER_DESC wrapSamplerDesc;
	D3D11_BUFFER_DESC blurTypeBufferDesc;

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

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture clamp sampler state description.
	clampSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	clampSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	clampSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	clampSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	clampSamplerDesc.MipLODBias = 0.0f;
	clampSamplerDesc.MaxAnisotropy = 1;
	clampSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	clampSamplerDesc.MinLOD = 0;
	clampSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	// Create the texture sampler state.
	renderer->CreateSamplerState(&clampSamplerDesc, &clampSampleState);

	// Create a texture wrap sampler state description.
	wrapSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	wrapSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	wrapSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	wrapSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	wrapSamplerDesc.MipLODBias = 0.0f;
	wrapSamplerDesc.MaxAnisotropy = 1;
	wrapSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	wrapSamplerDesc.BorderColor[0] = 0;
	wrapSamplerDesc.BorderColor[1] = 0;
	wrapSamplerDesc.BorderColor[2] = 0;
	wrapSamplerDesc.BorderColor[3] = 0;
	wrapSamplerDesc.MinLOD = 0;
	wrapSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	// Create the texture sampler state.
	renderer->CreateSamplerState(&wrapSamplerDesc, &wrapSampleState);

	// Setup the description of the screen size.
	blurTypeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	blurTypeBufferDesc.ByteWidth = sizeof(BlurTypeBufferType);
	blurTypeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	blurTypeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	blurTypeBufferDesc.MiscFlags = 0;
	blurTypeBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&blurTypeBufferDesc, NULL, &blurTypeBuffer);
}


void TextureShader::setShaderParameters(BasicShaderParameters basicShaderParameters, BlurType blurType)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(basicShaderParameters.worldMatrix);
	tview = XMMatrixTranspose(basicShaderParameters.viewMatrix);
	tproj = XMMatrixTranspose(basicShaderParameters.projectionMatrix);

	// Send basic matrix data
	result = basicShaderParameters.deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	basicShaderParameters.deviceContext->Unmap(matrixBuffer, 0);
	basicShaderParameters.deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// send blur type
	BlurTypeBufferType* blurTypePtr = nullptr;
	result = basicShaderParameters.deviceContext->Map(blurTypeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	blurTypePtr = (BlurTypeBufferType*)mappedResource.pData;
	blurTypePtr->blurType = static_cast<float>(blurType);
	blurTypePtr->padding = XMFLOAT3(0.0f, 0.0f, 0.0f);
	basicShaderParameters.deviceContext->Unmap(blurTypeBuffer, 0);
	basicShaderParameters.deviceContext->PSSetConstantBuffers(0, 1, &blurTypeBuffer);

	// Set shader texture and sampler resource in the pixel shader.
	basicShaderParameters.deviceContext->PSSetShaderResources(0, 1, &(basicShaderParameters.texture));

	if(blurType == BlurType::None) // use the clamp for normal texturing
		basicShaderParameters.deviceContext->PSSetSamplers(0, 1, &clampSampleState);
	else // use the wramp if we need to blur the img
		basicShaderParameters.deviceContext->PSSetSamplers(0, 1, &wrapSampleState);
}





