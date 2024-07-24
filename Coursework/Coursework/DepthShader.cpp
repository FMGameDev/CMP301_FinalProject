#include "DepthShader.h"

#include "NormalCalcTypeBufferType.h"

DepthShader::DepthShader(ID3D11Device* device, HWND hwnd, bool inApplyDisplacementMapping) :
	applyDisplacementMapping(inApplyDisplacementMapping), BaseShader(device, hwnd)
{
	if (!applyDisplacementMapping)
	{
		initShader(L"depth_vs.cso", L"depth_ps.cso");
	}
	else
	{
		initShader(L"depthDispMap_vs.cso", L"depth_ps.cso");
	}
}

DepthShader::~DepthShader()
{
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


void DepthShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;

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

void DepthShader::setShaderParameters(BasicShaderParameters& basicShaderParameters)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(basicShaderParameters.worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(basicShaderParameters.viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(basicShaderParameters.projectionMatrix);

	// Lock the constant buffer so it can be written to.
	basicShaderParameters.deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;

	basicShaderParameters.deviceContext->Unmap(matrixBuffer, 0);
	basicShaderParameters.deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);


	// send displacement map texture to the vertex shader
	if (applyDisplacementMapping)
	{
		basicShaderParameters.deviceContext->VSSetShaderResources(0, 1, &(basicShaderParameters.texture));
		basicShaderParameters.deviceContext->VSSetSamplers(0, 1, &sampleState); // texture for displacement mapping
	}
}