#define _USE_MATH_DEFINES
#include <cmath>
#include "Utils.h"

XMFLOAT3 Utils::calcDirectionFromAngles(float pitch, float yaw)
{
	XMFLOAT3 direction;
	direction.x = cos(yaw) * cos(pitch);
	direction.y = sin(pitch);
	direction.z = sin(yaw) * cos(pitch);

	return direction;
}

XMFLOAT2 Utils::calcAnglesFromDirection(XMFLOAT3 direction)
{
	// Following the inverse of calcDirectionFromAngles formula
    float pitch = asin(direction.y);
    float yaw = acos(direction.x / cos(pitch));

	return XMFLOAT2(pitch, yaw);
}

float Utils::degreesToRads(float degrees)
{
	float rads = (degrees * M_PI) / 180.0f;
	return rads;
}

float Utils::radsToDegrees(float rads)
{
	float degrees = (rads * 180.0f) / M_PI;
	return degrees;
}

XMFLOAT3 Utils::getCircunferencePoint(XMFLOAT3 origin, float radius, float angle)
{
	XMFLOAT3 pos; // point on the circumference

	angle = Utils::degreesToRads(angle); // degrees to rads

	// Equation derived from the Parametric Equation of a circle
	// where:
	//		- pos: any point on circumference
	//		- angle: angle made by the point with x-axis
	//		- radius: radius of the circumference
	//		- origin: center of the circumference
	pos.x = origin.x + (radius * cos(angle));
	pos.y = origin.y;
	pos.z = origin.z + (radius * sin(angle));

	return pos;
}

XMFLOAT4 Utils::rgbScale(XMFLOAT4 rgb)
{
	XMFLOAT4 rgbScaled = XMFLOAT4(rgb.x / 255.0f, rgb.y / 255.0f, rgb.z / 255.0f, rgb.w);

	return  rgbScaled;
}
