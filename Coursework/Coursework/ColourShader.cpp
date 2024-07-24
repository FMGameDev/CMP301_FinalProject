#include "ColourShader.h"

ColourShader::ColourShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"brightColour_vs.cso", L"brightColour_ps.cso");
}


ColourShader::~ColourShader()
{

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	if (colourBuffer)
	{
		colourBuffer->Release();
		colourBuffer = 0;
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


void ColourShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC colourBufferDesc;

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

	// Setup the description of the screen size.
	colourBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	colourBufferDesc.ByteWidth = sizeof(ColourBufferType);
	colourBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	colourBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	colourBufferDesc.MiscFlags = 0;
	colourBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&colourBufferDesc, NULL, &colourBuffer);
}


void ColourShader::setShaderParameters(BasicShaderParameters basicShaderParameters, XMFLOAT4 colour)
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

	// send colour type
	ColourBufferType* colourPtr = nullptr;
	result = basicShaderParameters.deviceContext->Map(colourBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	colourPtr = (ColourBufferType*)mappedResource.pData;
	colourPtr->colour = colour;
	basicShaderParameters.deviceContext->Unmap(colourBuffer, 0);
	basicShaderParameters.deviceContext->PSSetConstantBuffers(0, 1, &colourBuffer);

	// Set shader texture and sampler resource in the pixel shader.
	basicShaderParameters.deviceContext->PSSetShaderResources(0, 1, &(basicShaderParameters.texture));
}





