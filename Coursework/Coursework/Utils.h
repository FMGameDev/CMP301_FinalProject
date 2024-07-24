#pragma once
#include <directxmath.h>

using namespace DirectX;

class Utils
{
public:
	// Calculate the direction vector using the pitch and yaw angles (in radians)
	static XMFLOAT3 calcDirectionFromAngles(float pitch, float yaw);
	// Calculate the pitch and yaw angles (in radians) using the direction vector
	static XMFLOAT2 calcAnglesFromDirection(XMFLOAT3 direction);

	// Convert degrees to radians
	static float degreesToRads(float degrees);
	// Convert radians to degrees
	static float radsToDegrees(float rads);

	// Return a point in a circuference made by the parameters passed
	static XMFLOAT3 getCircunferencePoint(XMFLOAT3 origin, float radius, float angle);

	// Scale the rgb values from (0-255) to (0-1)
	static XMFLOAT4 rgbScale(XMFLOAT4 rgb);
};

