#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

struct BasicShaderParameters
{
	BasicShaderParameters(ID3D11DeviceContext* inDeviceContext, XMMATRIX inWorldMatrix, XMMATRIX inViewMatrix, XMMATRIX inProjectionMatrix, ID3D11ShaderResourceView* inTexture = nullptr) :
		deviceContext(inDeviceContext), worldMatrix(inWorldMatrix), viewMatrix(inViewMatrix), projectionMatrix(inProjectionMatrix), texture(inTexture) {}

	ID3D11DeviceContext* deviceContext;
	XMMATRIX worldMatrix;
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
	ID3D11ShaderResourceView* texture;
};