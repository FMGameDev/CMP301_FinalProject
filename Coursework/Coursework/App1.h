// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h" 	// include dxframework
using namespace std;
using namespace DirectX;

#include "DepthShader.h"
#include "BWShadowShader.h"
#include "LightShadowShader.h"
#include "TextureShader.h"
#include "GlowShader.h"
#include "ColourShader.h"
#include "BrightShader.h"
#include "PostProcessingShader.h"

#include "LightUtils.h"

#include <functional>
#include <vector>

struct BlurRenderTextures
{
	RenderTexture* downSampleTexture; // First step to blur a texture

	RenderTexture* horizontalBlurTexture;
	RenderTexture* verticalBlurTexture;

	RenderTexture* upSampleTexture; // Final texture blurred
};

struct ShadowLightRenderTextures
{
	ShadowMap* depthMap; // it will containt the depth map

	RenderTexture* blackWhiteRenderTexture; // it will contain black and white pixels according to the depth map

	BlurRenderTextures blurRenderTextures; // the upSample will contain the black and white render texture blurred
};

struct GlowRenderTextures
{
	RenderTexture* brightRenderTexture; // texture which will contain the result of the bright pass

	BlurRenderTextures blurRenderTextures; // the upSample will contain the bright render texture blurred
};

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool update();
	bool render();
	void gui();
	void guiLightCheckbox(Light* light, bool* lightIsOn, std::string lightId);
	void guiLightStats(Light* light, bool lightIsOn, std::string lightId, LightType lightType);

private:
	void initRenderTextures();
	void initBlurRenderTextures(BlurRenderTextures* blurRenderTextures);
	void initShadowLightRenderTextures(ShadowLightRenderTextures* shadowLightsRenderTextures);

	void initLights();
	void initMeshes();
	void initShaders();

	////////////////////////////////////////////////////////////////////
	//////////// GLOW PASS ////////////////////////////////
	void brightPass();

	////////////////////////////////////////////////////////////////////
	//////////// LIGHT DEPTH PASSES////////////////////////////////////
	
	// This function is where we render the scene to texture to perform the basic shadowing (depth passes)
	typedef std::function<void(ShadowLightRenderTextures*, bool, XMMATRIX, XMMATRIX)> FunctionPass;
	// Declare Lambdas
	FunctionPass depthPass = [&](ShadowLightRenderTextures* renderTextureShadowLights, bool lightIsOn, XMMATRIX lightViewMatrix, XMMATRIX lightProjectionMatrix) {
		this->lightDepthPass(renderTextureShadowLights, lightIsOn, lightViewMatrix, lightProjectionMatrix);
	};
	FunctionPass bwPass = [&](ShadowLightRenderTextures* renderTextureShadowLights, bool lightIsOn, XMMATRIX lightViewMatrix, XMMATRIX lightProjectionMatrix) {
		this->lightBlackWhitePass(renderTextureShadowLights, lightIsOn, lightViewMatrix, lightProjectionMatrix);
	};

	// function for performing passes for all the light shadows
	// the depth or black/white pass, depending the function passed
	void performLightPasses(FunctionPass func);
	void lightDepthPass(ShadowLightRenderTextures* renderTextureShadowLights, bool lightIsOn, XMMATRIX lightViewMatrix, XMMATRIX lightProjectionMatrix);
	void lightBlackWhitePass(ShadowLightRenderTextures* renderTextureShadowLights, bool lightIsOn, XMMATRIX lightViewMatrix, XMMATRIX lightProjectionMatrix);

	// Passes for each type of light //
	// Pass for directional light, it is used by depth and black/white passes
	void directionalLightPass(ShadowLightRenderTextures* renderTextureShadowLights, Light* light, FunctionPass func);
	// Pass for point light, it is used by depth and black/white passes
	void pointLightPass(ShadowLightRenderTextures* renderTextureShadowLights, Light* light, FunctionPass func);
	// Pass for spot light, it is used by depth and black/white passes
	void spotLightPass(ShadowLightRenderTextures* renderTextureShadowLights, Light* light, FunctionPass func);

	////////////////////////////////////////////////////////////////////
	//////////// BLUR PASS ////////////////////////////////
	void blurBWShadowTextures();
	void performBlurring(RenderTexture* initialTextureToBlur, BlurRenderTextures* blurRenderTextures, const int numTimesBlur = 1);
	void renderToTexturePass(RenderTexture* fromRenderTexture, RenderTexture* toRenderTexture, OrthoMesh* orthoMesh, BlurType blurType);

	////////////////////////////////////////////////////////////////////
	//////////// FINAL PASS ////////////////////////////////

	// Final Pass for rendering the objects to screen
	void finalPass();

	void preProcessing();
	void processing();
	void postProcessing();

private:


	// Objects //
	PlaneMesh* mountainMesh; // mesh used for displacement map
	std::vector<std::pair<SphereMesh*, float*>> sphereMeshes;
	AModel* model;

	// Meshes where the light sources (point and spotlight) are positioned
	CubeMesh* pLightMesh;
	CubeMesh* sLightMesh;

	XMFLOAT3 origin = XMFLOAT3(0.0f, 45.0f, 40.0f); // center point of a circle where the spheres will rotate
	float radius = 15.0f; // radius of the circunference

	// Collection of lights of the scene
	Lights lights;

	// Render textures for shadow lights
	ShadowLightRenderTextures dLightsRenderTextures[kDirectionalLightNum]; // for directional lights
	ShadowLightRenderTextures sLightsRenderTextures[kSpotLightNum]; // for spot light
	ShadowLightRenderTextures pLightsRenderTextures[kPointLightNum * kPointLightFacesNum]; // for point light, each six elements is one point light (is rendered as a cube)

	// Render textures used for post processing
	RenderTexture* processingSceneRenderTexture; // texture where will be save a full scene before the post-processing
	RenderTexture* postProcessingSceneRenderTexture;
	GlowRenderTextures glowRenderTextures;


	// Shader Objects
	DepthShader* depthShader;
	DepthShader* depthShaderDispMap;

	BWShadowShader* bwShadowShader;
	BWShadowShader* bwShadowShaderDispMap;

	TextureShader* textureShader;
	ColourShader* colourShader;

	LightShadowShader* lightShadowShader;
	LightShadowShader* lightShadowShaderDispMap;

	BrightShader* brightShader; // for glowing

	PostProcessingShader* postProcessingShader; // for put together all the scene (the processed and postprocessed)

	// Variables for defining shadow map
	const int SHADOW_MAP_WIDTH = 1024;
	const int SHADOW_MAP_HEIGHT = 1024;

	const float sceneWidth = 100;
	const float sceneHeight = 100;
	float shadowNear = 0.1f;
	float shadowFar = 100.0f;

	float lightAngle = 240.0f;
	float lightPosX = 9.0f;

	// Ortho meshes for render to textures
	OrthoMesh* orthoMeshDown;
	OrthoMesh* orthoMeshUp;

	// Variables for modifying the displacement mapping
	bool renderNormalColour;
	bool calculateNormalOnPS;
	bool calculateAvgNormal;
};

#endif