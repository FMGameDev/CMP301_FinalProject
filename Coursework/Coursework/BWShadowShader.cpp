#include "BWShadowShader.h"

BWShadowShader::BWShadowShader(ID3D11Device* device, HWND hwnd, bool inApplyDisplacementMapping) :
	applyDisplacementMapping(inApplyDisplacementMapping), BaseShader(device, hwnd)
{
	if (!applyDisplacementMapping)
	{
		initShader(L"bwShadow_vs.cso", L"bwShadow_ps.cso");
	}
	else
	{
		initShader(L"bwShadowDispMap_vs.cso", L"bwShadow_ps.cso");
	}
}


BWShadowShader::~BWShadowShader()
{
	// Release the sampler texture state.
	if (sampleStateShadow)
	{
		sampleStateShadow->Release();
		sampleStateShadow = 0;
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

	// Release the light constant buffer.
	if ( lightStateBuffer)
	{
		 lightStateBuffer->Release();
		 lightStateBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void BWShadowShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerShadowDesc;
	D3D11_SAMPLER_DESC samplerDesc; // wrap

	D3D11_BUFFER_DESC lightStateBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;

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


	// Sampler for shadow map sampling.
	samplerShadowDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerShadowDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerShadowDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerShadowDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerShadowDesc.MipLODBias = 0.0f;
	samplerShadowDesc.MaxAnisotropy = 1;
	samplerShadowDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerShadowDesc.BorderColor[0] = 1.0f;
	samplerShadowDesc.BorderColor[1] = 1.0f;
	samplerShadowDesc.BorderColor[2] = 1.0f;
	samplerShadowDesc.BorderColor[3] = 1.0f;
	samplerShadowDesc.MinLOD = 0;
	samplerShadowDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerShadowDesc, &sampleStateShadow);

	// Setup light buffer
	// Setup the description of the light dynamic constant buffer that is in the pixel shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	 lightStateBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	 lightStateBufferDesc.ByteWidth = sizeof(LightStateBufferType);
	 lightStateBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	 lightStateBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	 lightStateBufferDesc.MiscFlags = 0;
	 lightStateBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(& lightStateBufferDesc, NULL, & lightStateBuffer);

	// Create a texture sampler state description for using it in the texture of displacement mapping
	if (applyDisplacementMapping)
	{
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		renderer->CreateSamplerState(&samplerDesc, &sampleState);
	}
}


void BWShadowShader::setShaderParameters(BasicShaderParameters& basicShaderParameters, ID3D11ShaderResourceView* depthMapTexture, const XMMATRIX& lightViewMatrix, const XMMATRIX& lightProjectionMatrix, bool lightIsOn)
{
	// Variables to use along the funcion
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(basicShaderParameters.worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(basicShaderParameters.viewMatrix); // This is the cam view matrix
	XMMATRIX tproj = XMMatrixTranspose(basicShaderParameters.projectionMatrix);

	XMMATRIX tLightView = XMMatrixTranspose(lightViewMatrix);
	XMMATRIX tLightProj = XMMatrixTranspose(lightProjectionMatrix);

	// Lock the constant buffer so it can be written to.
	MatrixBufferType* dataPtr = nullptr;
	result = basicShaderParameters.deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	dataPtr->lightView = tLightView;
	dataPtr->lightProjection = tLightProj;
	basicShaderParameters.deviceContext->Unmap(matrixBuffer, 0);
	basicShaderParameters.deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);


	// Additional //

	// Lights: Send light data to pixel shader //
	LightStateBufferType* lightStatePtr = nullptr;
	result = basicShaderParameters.deviceContext->Map( lightStateBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightStatePtr = (LightStateBufferType*)mappedResource.pData;
	lightStatePtr->lightIsOn = (float)lightIsOn;
	lightStatePtr->padding = XMFLOAT3(0.0f, 0.0f, 0.0f);
	basicShaderParameters.deviceContext->Unmap( lightStateBuffer, 0);
	basicShaderParameters.deviceContext->PSSetConstantBuffers(0, 1, &lightStateBuffer);

	// Set shader texture resource in the pixel shader.
	basicShaderParameters.deviceContext->PSSetShaderResources(0, 1, &depthMapTexture); // this texture is the depth map made using the light passed as a parameter too
	basicShaderParameters.deviceContext->PSSetSamplers(0, 1, &sampleStateShadow);

	// send displacement map texture to the vertex shader
	if (applyDisplacementMapping)
	{
		basicShaderParameters.deviceContext->VSSetShaderResources(0, 1, &(basicShaderParameters.texture)); // this texture will be used for the displacement mapping
		basicShaderParameters.deviceContext->VSSetSamplers(0, 1, &sampleState); // texture for displacement mapping
	}
}