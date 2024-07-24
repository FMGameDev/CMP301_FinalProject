// Light class
// Holds data that represents a single light source
#include "light.h"
#include "../Coursework/Utils.h"

// create view matrix, based on light position and lookat. Used for shadow mapping.
void Light::generateViewMatrix()
{
	// default up vector
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	if (direction.y == 1 || (direction.x == 0 && direction.z == 0))
	{
		up = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0);
	}
	else if (direction.y == -1 || (direction.x == 0 && direction.z == 0))
	{
		up = XMVectorSet(0.0f, 0.0f, -1.0f, 1.0);
	}
	//XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	XMVECTOR dir = XMVectorSet(direction.x, direction.y, direction.z, 1.0f);
	XMVECTOR right = XMVector3Cross(dir, up);
	up = XMVector3Cross(right, dir);
	XMVECTOR lookAt = position + dir;
	// Create the view matrix from the three vectors.
	viewMatrix = XMMatrixLookAtLH(position, lookAt, up);
}

// Create a projection matrix for the (point) light source. Used in shadow mapping.
void Light::generateProjectionMatrix(float screenNear, float screenFar)
{
	float fieldOfView, screenAspect;

	// Setup field of view and screen aspect for a square light source.
	fieldOfView = (float)XM_PI / 2.0f;
	screenAspect = 1.0f;

	// Create the projection matrix for the light.
	projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenFar);
}

// Create orthomatrix for (directional) light source. Used in shadow mapping.
void Light::generateOrthoMatrix(float screenWidth, float screenHeight, float near, float far)
{
	orthoMatrix = XMMatrixOrthographicLH(screenWidth, screenHeight, near, far);
}

void Light::setAmbientColour(float red, float green, float blue, float alpha)
{
	ambientColour = XMFLOAT4(red, green, blue, alpha);
}

void Light::setDiffuseColour(float red, float green, float blue, float alpha)
{
	diffuseColour = XMFLOAT4(red, green, blue, alpha);
}

void Light::setDirection(float x, float y, float z)
{
	// save the direction
	direction = XMFLOAT3(x, y, z);

	// save the angles of that direction
	angles = Utils::calcAnglesFromDirection(direction);
}

void Light::setSpecularColour(float red, float green, float blue, float alpha)
{
	specularColour = XMFLOAT4(red, green, blue, alpha);
}

void Light::setSpecularPower(float power)
{
	specularPower = power;
}

void Light::setAttenuation(float constant, float lineal, float quadratic)
{
	attenuation = XMFLOAT3(constant, lineal, quadratic);
}

void Light::setSpotExponent(float spotExp)
{
	spotExponent = spotExp;
}

void Light::setPosition(float x, float y, float z)
{
	position = XMVectorSet(x, y, z, 1.0f);
}

void Light::setLookAt(float x, float y, float z)
{
	lookAt = XMVectorSet(x, y, z, 1.0f);
}

void Light::setAngles(float pitch, float yaw)
{
	angles.x = pitch;
	angles.y = yaw;

	direction = Utils::calcDirectionFromAngles(angles.x, angles.y);
}

void Light::setIsOn(bool newIsOn)
{
	isOn = newIsOn;
}

XMFLOAT4 Light::getAmbientColour()
{
	return ambientColour;
}

XMFLOAT4 Light::getDiffuseColour()
{
	return diffuseColour;
}


XMFLOAT3 Light::getDirection()
{
	return direction;
}

XMFLOAT4 Light::getSpecularColour()
{
	return specularColour;
}


float Light::getSpecularPower()
{
	return specularPower;
}

XMFLOAT3 Light::getAttenuation()
{
	return attenuation;
}

float Light::getSpotExponent()
{
	return spotExponent;
}

XMFLOAT3 Light::getPosition()
{
	XMFLOAT3 temp(XMVectorGetX(position), XMVectorGetY(position), XMVectorGetZ(position));
	return temp;
}

XMMATRIX Light::getViewMatrix()
{
	return viewMatrix;
}

XMMATRIX Light::getProjectionMatrix()
{
	return projectionMatrix;
}

XMMATRIX Light::getOrthoMatrix()
{
	return orthoMatrix;
}

XMFLOAT2 Light::getAngles()
{
	return angles;
}

bool Light::getIsOn()
{
	return isOn;
}
