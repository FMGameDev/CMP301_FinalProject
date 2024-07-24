#include "BrightShader.h"

BrightShader::BrightShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"bright_vs.cso", L"bright_ps.cso");
}


BrightShader::~BrightShader()
{
	// Release the sampler state.
	if (clampSampleState)
	{
		clampSampleState->Release();
		clampSampleState = 0;
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


void BrightShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
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

}


void BrightShader::setShaderParameters(BasicShaderParameters basicShaderParameters)
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

	// Set shader texture and sampler resource in the pixel shader.
	basicShaderParameters.deviceContext->PSSetShaderResources(0, 1, &(basicShaderParameters.texture));

	basicShaderParameters.deviceContext->PSSetSamplers(0, 1, &clampSampleState);
}
