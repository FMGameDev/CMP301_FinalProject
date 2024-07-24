#include "LightShadowShader.h"
#include "NormalCalcTypeBufferType.h"

LightShadowShader::LightShadowShader(ID3D11Device* device, HWND hwnd, bool inApplyDisplacementMapping) :
	applyDisplacementMapping(inApplyDisplacementMapping), BaseShader(device, hwnd)
{
	if (!applyDisplacementMapping)
	{
		initShader(L"lightShadow_vs.cso", L"lightShadow_ps.cso");
	}
	else
	{
		initShader(L"lightShadowDispMap_vs.cso", L"lightShadowDispMap_ps.cso");
	}

}


LightShadowShader::~LightShadowShader()
{
	// Release the sampler texture state.
	if (sampleStateTexture)
	{
		sampleStateTexture->Release();
		sampleStateTexture = 0;
	}

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

	// Release the matrix constant buffer.
	if (lightsMatrixBuffer)
	{
		lightsMatrixBuffer->Release();
		lightsMatrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	// Release the light constant buffer.
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}
	// Release the camera constant buffer.
	if (cameraBuffer)
	{
		cameraBuffer->Release();
		cameraBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void LightShadowShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC lightsMatrixBufferDesc;
	D3D11_SAMPLER_DESC samplerTextDesc;
	D3D11_SAMPLER_DESC samplerShadowDesc;

	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC normalCalcBufferDesc;

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

	lightsMatrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightsMatrixBufferDesc.ByteWidth = sizeof(LightsMatrixBufferType);
	lightsMatrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightsMatrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightsMatrixBufferDesc.MiscFlags = 0;
	lightsMatrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightsMatrixBufferDesc, NULL, &lightsMatrixBuffer);

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
	renderer->CreateSamplerState(&samplerTextDesc, &sampleStateTexture);

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
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightsBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

	// Setup camera buffer
	// Setup the description of the camera dynamic constant buffer that is in the vertex shader.
	// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);


	if (applyDisplacementMapping)
	{
		// Setup normal Calculation buffer
		// Setup the description of the light dynamic constant buffer that is in the pixel shader.
		// Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
		normalCalcBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		normalCalcBufferDesc.ByteWidth = sizeof(NormalCalcTypeBufferType);
		normalCalcBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		normalCalcBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		normalCalcBufferDesc.MiscFlags = 0;
		normalCalcBufferDesc.StructureByteStride = 0;
		renderer->CreateBuffer(&normalCalcBufferDesc, NULL, &normalCalcTypeBuffer);
	}
	
}


void LightShadowShader::setShaderParameters(BasicShaderParameters& basicShaderParameters, ShadowShaderParameters& shadowShaderParameters,
	Lights& lights, Camera* camera, NormalCalcTypeBufferType displacementMappingBufferType)
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


	LightsMatrixBufferType* lightsMatrixPtr = nullptr;
	result = basicShaderParameters.deviceContext->Map(lightsMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightsMatrixPtr = (LightsMatrixBufferType*)mappedResource.pData;
	// for point light matrices
	for (int lightId = 0; lightId < kPointLightNum; lightId++)
	{
		for (int faceId = 0; faceId < kPointLightFacesNum; faceId++)
		{
			switch (faceId)
			{
			case 0:
				// right
				lights.pLights[lightId]->setDirection(1.0f, 0.0f, 0.0f);
				break;
			case 1:
				// left
				lights.pLights[lightId]->setDirection(-1.0f, 0.0f, 0.0f);
				break;
			case 2:
				// up
				lights.pLights[lightId]->setDirection(0.0f, 1.0f, 0.0f);
				break;
			case 3:
				// down
				lights.pLights[lightId]->setDirection(0.0f, -1.0f, 0.0f);
				break;
			case 4:
				// front
				lights.pLights[lightId]->setDirection(0.0f, 0.0f, 1.0f);
				break;
			case 5:
				// back
				lights.pLights[lightId]->setDirection(0.0f, 0.0f, -1.0f);
				break;
			default:
				// back to state
				lights.pLights[lightId]->setDirection(0.0f, 0.0f, 0.0f);
				break;
			}

			// get the world, view, and projection matrices from the camera and d3d objects.
			lights.pLights[lightId]->generateViewMatrix();
			lights.pLights[lightId]->generateProjectionMatrix(shadowShaderParameters.shadowNear, shadowShaderParameters.shadowFar);

			lightsMatrixPtr->pLightViews[faceId + (lightId * kPointLightFacesNum)] = XMMatrixTranspose(lights.pLights[lightId]->getViewMatrix());
			lightsMatrixPtr->pLightProjections[faceId + (lightId * kPointLightFacesNum)] = XMMatrixTranspose(lights.pLights[lightId]->getProjectionMatrix());
		}
	}
	// for directional light matrices
	for (int i = 0; i < kDirectionalLightNum; i++)
	{
		lightsMatrixPtr->dLightViews[i] = XMMatrixTranspose(lights.dLights[i]->getViewMatrix());
		lightsMatrixPtr->dLightProjections[i] = XMMatrixTranspose(lights.dLights[i]->getOrthoMatrix());
	}
	// for spot light matrices
	for (int i = 0; i < kSpotLightNum; i++)
	{
		lightsMatrixPtr->sLightViews[i] = XMMatrixTranspose(lights.sLights[i]->getViewMatrix());
		lightsMatrixPtr->sLightProjections[i] = XMMatrixTranspose(lights.sLights[i]->getOrthoMatrix());
	}
	basicShaderParameters.deviceContext->Unmap(lightsMatrixBuffer, 0);
	basicShaderParameters.deviceContext->VSSetConstantBuffers(1, 1, &lightsMatrixBuffer);


	// Camera: Send camera data to pixel shader //
	CameraBufferType* cameraPtr = nullptr;
	result = basicShaderParameters.deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraPtr = (CameraBufferType*)mappedResource.pData;
	cameraPtr->camera_pos = camera->getPosition();
	cameraPtr->padding = (float)0.0f;
	basicShaderParameters.deviceContext->Unmap(cameraBuffer, 0);
	basicShaderParameters.deviceContext->VSSetConstantBuffers(2, 1, &cameraBuffer);


	// Lights characteristics: Send light data to pixel shader //
	LightsBufferType* lightPtr = nullptr;
	result = basicShaderParameters.deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightsBufferType*)mappedResource.pData;
	// Set Directional Light
	for (int i = 0; i < kDirectionalLightNum; i++)
	{
		lightPtr->dLights[i].ambient = lights.dLights[i]->getAmbientColour();
		lightPtr->dLights[i].diffuse = lights.dLights[i]->getDiffuseColour();
		lightPtr->dLights[i].specular = lights.dLights[i]->getSpecularColour();
		lightPtr->dLights[i].direction = lights.dLights[i]->getDirection();
		lightPtr->dLights[i].specularPower = lights.dLights[i]->getSpecularPower();
		lightPtr->dLights[i].padding = XMFLOAT3(0.0f, 0.0f, 0.0f);
		lightPtr->dLights[i].isOn = (float)lights.dLights[i]->getIsOn();
	}
	// Set Point Light
	for (int i = 0; i < kPointLightNum; i++)
	{
		lightPtr->pLights[i].ambient = lights.pLights[i]->getAmbientColour();
		lightPtr->pLights[i].diffuse = lights.pLights[i]->getDiffuseColour();
		lightPtr->pLights[i].specular = lights.pLights[i]->getSpecularColour();
		lightPtr->pLights[i].position = lights.pLights[i]->getPosition();
		lightPtr->pLights[i].specularPower = lights.pLights[i]->getSpecularPower();
		lightPtr->pLights[i].attenuation = lights.pLights[i]->getAttenuation();
		lightPtr->pLights[i].isOn = (float)lights.pLights[i]->getIsOn();
	}
	// Set SpotLight
	for (int i = 0; i < kSpotLightNum; i++)
	{
		lightPtr->sLights[i].ambient = lights.sLights[i]->getAmbientColour();
		lightPtr->sLights[i].diffuse = lights.sLights[i]->getDiffuseColour();
		lightPtr->sLights[i].specular = lights.sLights[i]->getSpecularColour();
		lightPtr->sLights[i].position = lights.sLights[i]->getPosition();
		lightPtr->sLights[i].specularPower = lights.sLights[i]->getSpecularPower();
		lightPtr->sLights[i].direction = lights.sLights[i]->getDirection();
		lightPtr->sLights[i].spotExponent = lights.sLights[i]->getSpotExponent();
		lightPtr->sLights[i].attenuation = lights.sLights[i]->getAttenuation();
		lightPtr->sLights[i].isOn = (float)lights.sLights[i]->getIsOn();
	}
	basicShaderParameters.deviceContext->Unmap(lightBuffer, 0);
	basicShaderParameters.deviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);

	// Set shader texture resource in the pixel shader.
	basicShaderParameters.deviceContext->PSSetShaderResources(0, 1, &(basicShaderParameters.texture));
	basicShaderParameters.deviceContext->PSSetShaderResources(1, kDirectionalLightNum, shadowShaderParameters.dLightShadowTextures);
	basicShaderParameters.deviceContext->PSSetShaderResources(1 + kDirectionalLightNum, kSpotLightNum, shadowShaderParameters.sLightShadowTextures);
	basicShaderParameters.deviceContext->PSSetShaderResources(1 + kDirectionalLightNum + kSpotLightNum, kPointLightNum * kPointLightFacesNum, shadowShaderParameters.pLightShadowTextures);

	basicShaderParameters.deviceContext->PSSetSamplers(0, 1, &sampleStateTexture);
	basicShaderParameters.deviceContext->PSSetSamplers(1, 1, &sampleStateShadow);

	// TO VS and PS
	if (applyDisplacementMapping)
	{
		NormalCalcTypeBufferType* displacementMappingPtr;
		basicShaderParameters.deviceContext->Map(normalCalcTypeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		displacementMappingPtr = (NormalCalcTypeBufferType*)mappedResource.pData;
		displacementMappingPtr->calcNormalOnPS = displacementMappingBufferType.calcNormalOnPS;
		displacementMappingPtr->calcAvgNormal = displacementMappingBufferType.calcAvgNormal;
		displacementMappingPtr->renderNormalColour = displacementMappingBufferType.renderNormalColour;
		displacementMappingPtr->padding1 = displacementMappingBufferType.padding1;
		displacementMappingPtr->realWorldDimension = displacementMappingBufferType.realWorldDimension;
		displacementMappingPtr->padding2 = displacementMappingBufferType.padding2;

		basicShaderParameters.deviceContext->Unmap(normalCalcTypeBuffer, 0);
		basicShaderParameters.deviceContext->VSSetConstantBuffers(3, 1, &normalCalcTypeBuffer);
		basicShaderParameters.deviceContext->PSSetConstantBuffers(1, 1, &normalCalcTypeBuffer);
	}
}