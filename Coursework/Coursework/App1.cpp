#include "App1.h"
#include "Utils.h"
#include "BasicShaderParameters.h"
#include "ShadowShaderParameters.h"

App1::App1()
{
	// Shaders
	lightShadowShader = nullptr;
	lightShadowShaderDispMap = nullptr;

	// objects
	mountainMesh = nullptr;

	// meshes where the light source is
	pLightMesh = nullptr;
	sLightMesh = nullptr;
}

App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.
	for (Light* light : lights.dLights)
	{
		delete light;
		light = nullptr;
	}

	for (Light* light : lights.pLights)
	{
		delete light;
		light = nullptr;
	}

	for (Light* light : lights.sLights)
	{
		delete light;
		light = nullptr;
	}
}


// Init functions //

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	initMeshes();
	initLights();
	initShaders();

	// load texture
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"height_map", L"res/height.png");

	// Init render textures
	initRenderTextures();

	// initialise camera
	camera->setPosition(-2.0f, 20.0f, -30.0f);
}


void App1::initLights()
{
	// Initialise lights //
	lights.dLights[0] = new Light();
	lights.dLights[0]->setAmbientColour(0.1f, 0.1f, 0.1f, 1.0f); // it should be a range value [0.1, 0.3]
	lights.dLights[0]->setDiffuseColour(0.0f, 0.0f, 1.0f, 1.0f); // blue
	lights.dLights[0]->setSpecularColour(0.0f, 0.0f, 1.0f, 1.0f);
	lights.dLights[0]->setAngles(Utils::degreesToRads(-83.0f), Utils::degreesToRads(0.0f));
	//lights.dLights[0]->setDirection(0.0f, -0.7f, -0.7f); // direction down onto the geometry
	lights.dLights[0]->setPosition(0.f, 75.f, 16.f);
	lights.dLights[0]->setSpecularPower(256.0f);
	lights.dLights[0]->setIsOn(true);

	lights.dLights[1] = new Light();
	lights.dLights[1]->setAmbientColour(0.1f, 0.1f, 0.1f, 1.0f); // it should be a range value [0.1, 0.3]
	lights.dLights[1]->setDiffuseColour(1.0f, 0.0f, 0.0f, 1.0f); // red
	lights.dLights[1]->setSpecularColour(1.0f, 0.0f, 0.0f, 1.0f);
	lights.dLights[1]->setAngles(Utils::degreesToRads(-47.0f), Utils::degreesToRads(0.0f));
	lights.dLights[1]->setPosition(-45.0f, 40.0f, 40.f);
	lights.dLights[1]->setSpecularPower(256.0f);
	lights.dLights[1]->setIsOn(true);

	lights.pLights[0] = new Light();
	lights.pLights[0]->setAmbientColour(0.1f, 0.1f, 0.1f, 1.0f); // it should be a range value [0.1, 0.3]
	lights.pLights[0]->setDiffuseColour(0.2f, 1.0f, 0.2f, 1.0f); // green
	lights.pLights[0]->setSpecularColour(0.2f, 1.0f, 0.2f, 1.0f);
	lights.pLights[0]->setPosition(0.0f, 62.0f, 40.0f);
	lights.pLights[0]->setSpecularPower(128.0f);
	lights.pLights[0]->setAttenuation(1.0f, 0.124f, 0.005f);
	lights.pLights[0]->setIsOn(true);

	lights.sLights[0] = new Light();
	lights.sLights[0]->setAmbientColour(0.01f, 0.01f, 0.01f, 1.0f); // it should be a range value [0.2, 0.3]
	lights.sLights[0]->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f); // white
	lights.sLights[0]->setSpecularColour(1.0f, 1.0f, 1.0f, 1.0f);
	lights.sLights[0]->setPosition(25.0f, 33.0f, 64.0f); // as it is a spot, it has a position
	lights.sLights[0]->setSpecularPower(256.0f);
	lights.sLights[0]->setAngles(Utils::degreesToRads(-182.0f), Utils::degreesToRads(79.0f)); // direction
	lights.sLights[0]->setSpotExponent(5.0f);
	lights.sLights[0]->setAttenuation(1.0f, 0.0f, 0.0f);
	lights.sLights[0]->setIsOn(true);


}


void App1::initRenderTextures()
{
	// Initialise renderer textures for lights //

	for (int i = 0; i < kDirectionalLightNum; i++)
	{
		initShadowLightRenderTextures(&dLightsRenderTextures[i]);
	}
	for (int lightId = 0; lightId < kPointLightNum; lightId++)
	{
		for (int faceId = 0; faceId < kPointLightFacesNum; faceId++)
		{
			initShadowLightRenderTextures(&pLightsRenderTextures[faceId + (lightId * kPointLightFacesNum)]);
		}
	}
	for (int i = 0; i < kSpotLightNum; i++)
	{
		initShadowLightRenderTextures(&sLightsRenderTextures[i]);
	}

	// Init render textures for post production
	processingSceneRenderTexture = new RenderTexture(renderer->getDevice(), sWidth, sHeight, SCREEN_NEAR, SCREEN_DEPTH); // full screen size
	postProcessingSceneRenderTexture = new RenderTexture(renderer->getDevice(), sWidth, sHeight, SCREEN_NEAR, SCREEN_DEPTH); // full screen size

	// Glow textures
	glowRenderTextures.brightRenderTexture = new RenderTexture(renderer->getDevice(), sWidth, sHeight, SCREEN_NEAR, SCREEN_DEPTH); // full screen size
	initBlurRenderTextures(&(glowRenderTextures.blurRenderTextures));
}

void App1::initBlurRenderTextures(BlurRenderTextures* blurRenderTextures)
{
	const int HALFSCREEN_WITH = sWidth / 2;
	const int HALFSCREEN_HEIGHT = sHeight / 2;

	blurRenderTextures->downSampleTexture = new RenderTexture(renderer->getDevice(), HALFSCREEN_WITH, HALFSCREEN_HEIGHT, SCREEN_NEAR, SCREEN_DEPTH);
	blurRenderTextures->horizontalBlurTexture = new RenderTexture(renderer->getDevice(), HALFSCREEN_WITH, HALFSCREEN_HEIGHT, SCREEN_NEAR, SCREEN_DEPTH);
	blurRenderTextures->verticalBlurTexture = new RenderTexture(renderer->getDevice(), HALFSCREEN_WITH, HALFSCREEN_HEIGHT, SCREEN_NEAR, SCREEN_DEPTH);
	blurRenderTextures->upSampleTexture = new RenderTexture(renderer->getDevice(), sWidth, sHeight, SCREEN_NEAR, SCREEN_DEPTH);
}

void App1::initShadowLightRenderTextures(ShadowLightRenderTextures* shadowLightsRenderTextures)
{
	const int HALFSCREEN_WITH = sWidth / 2;
	const int HALFSCREEN_HEIGHT = sHeight / 2;
	
	// Initialise textures specific textures for light shadows
	shadowLightsRenderTextures->depthMap = new ShadowMap(renderer->getDevice(), (float)SHADOW_MAP_WIDTH, (float)SHADOW_MAP_HEIGHT);
	shadowLightsRenderTextures->blackWhiteRenderTexture = new RenderTexture(renderer->getDevice(), sWidth, sHeight, SCREEN_NEAR, SCREEN_DEPTH);

	// Initialise render textures for blur
	initBlurRenderTextures(&(shadowLightsRenderTextures->blurRenderTextures));
}


void App1::initMeshes()
{
	// Create terrain:
	mountainMesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	model = new AModel(renderer->getDevice(), "res/teapot.obj");

	// Create spheres:
	float sphereAngle = 0.0f;
	for (int i = 0; i < 8; i++)
	{
		sphereMeshes.push_back(std::pair<SphereMesh*, float*>(new SphereMesh(renderer->getDevice(), renderer->getDeviceContext()), new float(sphereAngle)));
		sphereAngle += 45.0f;
	}

	// Create debug meshes where the spot and point lights will be:
	pLightMesh = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	sLightMesh = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());

	// Meshes which will be used for texture rendering or displaying to screen (debug)
	const int HALFSCREEN_WITH = sWidth / 2;
	const int HALFSCREEN_HEIGHT = sHeight / 2;
	orthoMeshDown = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), HALFSCREEN_WITH, HALFSCREEN_HEIGHT);	// 1/4 of screen size
	orthoMeshUp = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), sWidth, sHeight);	// same screen size
}

void App1::initShaders()
{
	// Initialise shaders //
	depthShader = new DepthShader(renderer->getDevice(), wnd);
	depthShaderDispMap = new DepthShader(renderer->getDevice(), wnd, true);
	lightShadowShader = new LightShadowShader(renderer->getDevice(), wnd);
	lightShadowShaderDispMap = new LightShadowShader(renderer->getDevice(), wnd, true);
	bwShadowShader = new BWShadowShader(renderer->getDevice(), wnd);
	bwShadowShaderDispMap = new BWShadowShader(renderer->getDevice(), wnd, true);
	textureShader = new TextureShader(renderer->getDevice(), wnd);
	colourShader = new ColourShader(renderer->getDevice(), wnd);

	brightShader = new BrightShader(renderer->getDevice(), wnd);
	postProcessingShader = new PostProcessingShader(renderer->getDevice(), wnd);
}



// Basic/main functions of App1 //

bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}

	// Update any variable on cpu
	result = update();
	if (!result)
	{
		return false;
	}

	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::update()
{
	// update camera
	camera->update();

	// Update the angle of the spheres on cpu
	float dt = timer->getTime();
	float sphereSpeed = 10.0f;

	for (std::pair<SphereMesh*, float*> sphere : sphereMeshes)
	{
		*sphere.second += (sphereSpeed * dt);

		if (*sphere.second >= 360.0f)
		{
			*sphere.second = 0.0f;
		}
	}

	return true;
}

bool App1::render()
{
	// PRE-PROCESSING //
	preProcessing();	

	// PROCESSING //
	processing();

	// POST-PROCESSING //
	postProcessing();

	// Render scene
	finalPass();

	return true;
}


// Pre-processing Functions //

void App1::preProcessing()
{
	// Lighting passes//

	// Firstly, render the scene to a texture (depth map of all the lights)
	performLightPasses(depthPass);
	// Secondly, transform that depth map textures to black and white textures
	performLightPasses(bwPass);

	// Next, blur all the black and white shadow textures
	// (Down sample, horizontal and vertical blur, and up sample)
	blurBWShadowTextures();
}

void App1::lightDepthPass(ShadowLightRenderTextures* renderTextureShadowLights, bool lightIsOn, XMMATRIX lightViewMatrix, XMMATRIX lightProjectionMatrix)
{
	// Set the render target to be the render to texture and clear it
	//renderTextureShadowLights->renderTexture->setRenderTarget(renderer->getDeviceContext());
	//renderTextureShadowLights->renderTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 0.0f, 0.0f, 1.0f);
	renderTextureShadowLights->depthMap->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	BasicShaderParameters basicShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	BasicShaderParameters basicShaderParametersDispMap(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix, textureMgr->getTexture(L"height_map"));

	// Render spheres from light perspective
	for (std::pair<SphereMesh*, float*> sphere : sphereMeshes)
	{
		XMFLOAT3 newPos = Utils::getCircunferencePoint(origin, radius, *sphere.second);
		XMMATRIX sphereWorldMatrix = worldMatrix * XMMatrixTranslation(newPos.x, newPos.y, newPos.z);
		basicShaderParameters.worldMatrix = sphereWorldMatrix;

		sphere.first->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(basicShaderParameters);
		depthShader->render(renderer->getDeviceContext(), sphere.first->getIndexCount());
	}

	// Render floor from light perspective
	XMMATRIX floorWorldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	basicShaderParametersDispMap.worldMatrix = floorWorldMatrix;

	mountainMesh->sendData(renderer->getDeviceContext());
	depthShaderDispMap->setShaderParameters(basicShaderParametersDispMap);
	depthShaderDispMap->render(renderer->getDeviceContext(), mountainMesh->getIndexCount());

	// Render model
	XMMATRIX modelWorldMatrix = worldMatrix * XMMatrixTranslation(80.f, 8.f, 0.f) * XMMatrixScaling(0.5f, 0.5f, 0.5f);
	basicShaderParameters.worldMatrix = modelWorldMatrix;

	model->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(basicShaderParameters);
	depthShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::lightBlackWhitePass(ShadowLightRenderTextures* renderTextureShadowLights, bool lightIsOn, XMMATRIX lightViewMatrix, XMMATRIX lightProjectionMatrix)
{
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX camViewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	ID3D11ShaderResourceView* lightShadowDepthMap = renderTextureShadowLights->depthMap->getDepthMapSRV();

	BasicShaderParameters basicShaderParameters(renderer->getDeviceContext(), worldMatrix, camViewMatrix, projectionMatrix);
	BasicShaderParameters basicShaderParametersDispMap(renderer->getDeviceContext(), worldMatrix, camViewMatrix, projectionMatrix, textureMgr->getTexture(L"height_map"));

	// Set the render target to be the render to texture and clear it
	renderTextureShadowLights->blackWhiteRenderTexture->setRenderTarget(renderer->getDeviceContext());
	renderTextureShadowLights->blackWhiteRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 1.0f, 0.0f, 1.0f);

	// Render spheres from light perspective
	for (std::pair<SphereMesh*, float*> sphere : sphereMeshes)
	{
		XMFLOAT3 newPos = Utils::getCircunferencePoint(origin, radius, *sphere.second);
		XMMATRIX sphereWorldMatrix = worldMatrix * XMMatrixTranslation(newPos.x, newPos.y, newPos.z);
		basicShaderParameters.worldMatrix = sphereWorldMatrix;

		sphere.first->sendData(renderer->getDeviceContext());
		bwShadowShader->setShaderParameters(basicShaderParameters, lightShadowDepthMap, lightViewMatrix, lightProjectionMatrix, lightIsOn);
		bwShadowShader->render(renderer->getDeviceContext(), sphere.first->getIndexCount());
	}

	// Render floor
	XMMATRIX floorWorldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	basicShaderParameters.worldMatrix = floorWorldMatrix;

	basicShaderParametersDispMap.worldMatrix = floorWorldMatrix;
	mountainMesh->sendData(renderer->getDeviceContext());
	bwShadowShaderDispMap->setShaderParameters(basicShaderParametersDispMap, lightShadowDepthMap, lightViewMatrix, lightProjectionMatrix, lightIsOn);
	bwShadowShaderDispMap->render(renderer->getDeviceContext(), mountainMesh->getIndexCount());

	// Render model
	XMMATRIX modelWorldMatrix = worldMatrix * XMMatrixTranslation(80.f, 8.f, 0.f) * XMMatrixScaling(0.5f, 0.5f, 0.5f);
	basicShaderParameters.worldMatrix = modelWorldMatrix;

	model->sendData(renderer->getDeviceContext());
	bwShadowShader->setShaderParameters(basicShaderParameters, lightShadowDepthMap, lightViewMatrix, lightProjectionMatrix, lightIsOn);
	bwShadowShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}


void App1::performLightPasses(FunctionPass func)
{
	// Perform depth or blackwhite passes
	for (int i = 0; i < kDirectionalLightNum; i++)
	{
		directionalLightPass(&dLightsRenderTextures[i], lights.dLights[i], func);
	}
	for (int i = 0; i < kSpotLightNum; i++)
	{
		spotLightPass(&sLightsRenderTextures[i], lights.sLights[i], func);
	}
	for (int lightId = 0; lightId < kPointLightNum; lightId++)
	{
		for (int faceId = 0; faceId < kPointLightFacesNum; faceId++)
		{
			pointLightPass(&pLightsRenderTextures[faceId + (lightId * kPointLightFacesNum)], lights.pLights[lightId], func);
		}
	}

}

void App1::directionalLightPass(ShadowLightRenderTextures* renderTextureShadowLights, Light* light, FunctionPass func)
{
	// get the world, view, and projection matrices from the camera and d3d objects.
	light->generateViewMatrix();
	light->generateOrthoMatrix(sceneWidth, sceneHeight, shadowNear, shadowFar);

	XMMATRIX lightViewMatrix = light->getViewMatrix();
	XMMATRIX lightProjectionMatrix = light->getOrthoMatrix();

	func(renderTextureShadowLights, light->getIsOn(), lightViewMatrix, lightProjectionMatrix);
}

void App1::pointLightPass(ShadowLightRenderTextures* renderTextureShadowLights, Light* light, FunctionPass func)
{
	// Set the render target to be the render to texture.
	for (int i = 0; i < kPointLightFacesNum; i++)
	{
		switch (i)
		{
		case 0:
			// right
			light->setDirection(1.0f, 0.0f, 0.0f);
			break;
		case 1:
			// left
			light->setDirection(-1.0f, 0.0f, 0.0f);
			break;
		case 2:
			// up
			light->setDirection(0.0f, 1.0f, 0.0f);
			break;
		case 3:
			// down
			light->setDirection(0.0f, -1.0f, 0.0f);
			break;
		case 4:
			// front
			light->setDirection(0.0f, 0.0f, 1.0f);
			break;
		case 5:
			// back
			light->setDirection(0.0f, 0.0f, -1.0f);
			break;
		default:
			// back to state
			light->setDirection(0.0f, 0.0f, 0.0f);
			break;
		}

		// generate the view, and projection matrices from the light with the new direction.
		light->generateViewMatrix();
		light->generateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);

		XMMATRIX lightViewMatrix = light->getViewMatrix();
		XMMATRIX lightProjectionMatrix = light->getProjectionMatrix();

		func(renderTextureShadowLights, light->getIsOn(), lightViewMatrix, lightProjectionMatrix);
	}
}

void App1::spotLightPass(ShadowLightRenderTextures* renderTextureShadowLights, Light* light, FunctionPass func)
{
	// generate the view, and projection matrices
	light->generateViewMatrix();
	light->generateOrthoMatrix(sceneWidth, sceneHeight, shadowNear, shadowFar);

	XMMATRIX lightViewMatrix = light->getViewMatrix();
	XMMATRIX lightProjectionMatrix = light->getOrthoMatrix();

	func(renderTextureShadowLights, light->getIsOn(), lightViewMatrix, lightProjectionMatrix);
}


void App1::blurBWShadowTextures()
{
	// Blur Light Shadows //
	
	// Blur directional light shadows
	for (int i = 0; i < kDirectionalLightNum; i++)
	{
		performBlurring(dLightsRenderTextures[i].blackWhiteRenderTexture, &(dLightsRenderTextures[i].blurRenderTextures), 2); // blur twice for getting a smoother shadow
	}

	// Blur spot light shadows
	for (int i = 0; i < kSpotLightNum; i++)
	{
		performBlurring(sLightsRenderTextures[i].blackWhiteRenderTexture, &(sLightsRenderTextures[i].blurRenderTextures), 2); // blur twice for getting a smoother shadow
	}

	// Blur point light shadows
	for (int lightId = 0; lightId < kPointLightNum; lightId++)
	{
		for (int faceId = 0; faceId < kPointLightFacesNum; faceId++)
		{
			performBlurring(pLightsRenderTextures[faceId + (lightId * kPointLightFacesNum)].blackWhiteRenderTexture , &(pLightsRenderTextures[faceId + (lightId * kPointLightFacesNum)].blurRenderTextures), 2); // blur twice for getting a smoother shadow
		}
	}

	// Blur the glowing
	//performBlurring(glowingRenderTextures.blackWhiteRenderTexture, &(glowingRenderTextures.blurRenderTextures));
}

void App1::performBlurring(RenderTexture* initialTextureToBlur, BlurRenderTextures* blurRenderTextures, int numTimesBlur)
{
	// Down sample the black and white scene textures.
	renderToTexturePass(initialTextureToBlur, blurRenderTextures->downSampleTexture, orthoMeshDown, BlurType::None);

	// Perform the actual blur x number of times
	for (int i = 0; i < numTimesBlur; i++)
	{
		// Perform horizontal blur on the down sampled textures
		renderToTexturePass(blurRenderTextures->downSampleTexture, blurRenderTextures->horizontalBlurTexture, orthoMeshDown, BlurType::Horizontal);
		// Perform vertical blur on the down sampled textures
		renderToTexturePass(blurRenderTextures->horizontalBlurTexture, blurRenderTextures->verticalBlurTexture, orthoMeshDown, BlurType::Vertical);

		// If there is at least another blur to do then set the downSampleTexture to the latest render texture which has been blurred and it has the same size
		if (numTimesBlur > 1 && i < numTimesBlur)
			blurRenderTextures->downSampleTexture = blurRenderTextures->verticalBlurTexture;
	}

	// Finally up sample the final blurred render to textures that can now be used in the light-shadow shader.
	renderToTexturePass(blurRenderTextures->verticalBlurTexture, blurRenderTextures->upSampleTexture, orthoMeshUp, BlurType::None);
}

void App1::renderToTexturePass(RenderTexture* fromRenderTexture, RenderTexture* toRenderTexture, OrthoMesh* orthoMesh, BlurType blurType)
{
	// Set the main parameters to send to the shaders
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering
	XMMATRIX orthoMatrix = toRenderTexture->getOrthoMatrix();  // ortho matrix for 2D rendering
	BasicShaderParameters basicShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, fromRenderTexture->getShaderResourceView());


	// Set the render target to be the render to texture and clear it
	toRenderTexture->setRenderTarget(renderer->getDeviceContext());
	toRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 0.0f, 1.0f);
	{
		// Render
		renderer->setZBuffer(false);
		{
			orthoMesh->sendData(renderer->getDeviceContext());
			textureShader->setShaderParameters(basicShaderParameters, blurType);
			textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
		}
		renderer->setZBuffer(true);
	}
	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}


// Processing is the almost final scene (without the post-processing passses) //
void App1::processing()
{
	// SET ALL THE PARAMETERS FOR THE PASS//
	
	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();
	// Set the main parameters to send to the shaders
	BasicShaderParameters basicShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"));
	BasicShaderParameters basicShaderParametersDispMap(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"height_map"));
	NormalCalcTypeBufferType noneNormalCalcTypeBuffer;
	NormalCalcTypeBufferType normalCalcTypeBuffer((float)calculateNormalOnPS, (float)calculateAvgNormal, (float)renderNormalColour, XMFLOAT2((float)mountainMesh->getResolution(), (float)mountainMesh->getResolution()));

	ShadowShaderParameters shadowShaderParameters;
	for (int i = 0; i < kDirectionalLightNum; i++)
	{
		shadowShaderParameters.dLightShadowTextures[i] = dLightsRenderTextures[i].blurRenderTextures.upSampleTexture->getShaderResourceView();
	}
	for (int i = 0; i < kSpotLightNum; i++)
	{
		shadowShaderParameters.sLightShadowTextures[i] = sLightsRenderTextures[i].blurRenderTextures.upSampleTexture->getShaderResourceView();
	}
	for (int lightId = 0; lightId < kPointLightNum; lightId++)
	{
		for (int faceId = 0; faceId < kPointLightFacesNum; faceId++)
		{
			shadowShaderParameters.pLightShadowTextures[faceId + (lightId * kPointLightFacesNum)] = pLightsRenderTextures[faceId + (lightId * kPointLightFacesNum)].blurRenderTextures.upSampleTexture->getShaderResourceView();
		}
	}
	shadowShaderParameters.shadowFar = SCREEN_DEPTH;
	shadowShaderParameters.shadowNear = SCREEN_NEAR;


	// PASS //

	// Set the render target to be the render to texture and clear it
	processingSceneRenderTexture->setRenderTarget(renderer->getDeviceContext());
	processingSceneRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);
	{
		// Render spheres //
		for (std::pair<SphereMesh*, float*> sphere : sphereMeshes)
		{
			// calculate world position for this sphere mesh
			XMFLOAT3 newPos = Utils::getCircunferencePoint(origin, radius, *sphere.second);
			XMMATRIX sMWorldMatrix = worldMatrix * XMMatrixTranslation(newPos.x, newPos.y, newPos.z);
			basicShaderParameters.worldMatrix = sMWorldMatrix;

			sphere.first->sendData(renderer->getDeviceContext());
			lightShadowShader->setShaderParameters(basicShaderParameters, shadowShaderParameters, lights, camera, noneNormalCalcTypeBuffer);
			lightShadowShader->render(renderer->getDeviceContext(), sphere.first->getIndexCount());
		}

		BasicShaderParameters glowBasicShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"glow_map"));

		// Point Light mesh //
		if (lights.pLights[0]->getIsOn() == true)
		{
			// calculate world position for this point light mesh
			XMFLOAT2 pointLightAngles = lights.pLights[0]->getAngles();
			XMFLOAT3 pointLightPos = lights.pLights[0]->getPosition();
			XMMATRIX pLWorldMatrix = worldMatrix * XMMatrixRotationRollPitchYaw(pointLightAngles.x, pointLightAngles.y, 1.0f) * XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(pointLightPos.x, pointLightPos.y, pointLightPos.z);
			glowBasicShaderParameters.worldMatrix = pLWorldMatrix;

			pLightMesh->sendData(renderer->getDeviceContext());
			colourShader->setShaderParameters(glowBasicShaderParameters, lights.pLights[0]->getDiffuseColour());
			colourShader->render(renderer->getDeviceContext(), pLightMesh->getIndexCount());
		}
		// Spot Light Mesh //
		if (lights.sLights[0]->getIsOn() == true)
		{
			// calculate world position for this spotlight mesh
			XMFLOAT2 spotLightAngles = lights.sLights[0]->getAngles();
			XMFLOAT3 spotLightPos = lights.sLights[0]->getPosition();
			XMMATRIX sLWorldMatrix = worldMatrix * XMMatrixRotationRollPitchYaw(spotLightAngles.x, spotLightAngles.y, 1.0f) * XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(spotLightPos.x, spotLightPos.y, spotLightPos.z);
			basicShaderParameters.worldMatrix = sLWorldMatrix;

			sLightMesh->sendData(renderer->getDeviceContext());
			textureShader->setShaderParameters(basicShaderParameters);
			textureShader->render(renderer->getDeviceContext(), sLightMesh->getIndexCount());
		}

		// Render floor Mesh //
		// calculate the floor world matrix
		XMMATRIX floorWorldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
		basicShaderParameters.worldMatrix = floorWorldMatrix;

		basicShaderParametersDispMap.worldMatrix = floorWorldMatrix;
		mountainMesh->sendData(renderer->getDeviceContext());
		lightShadowShaderDispMap->setShaderParameters(basicShaderParametersDispMap, shadowShaderParameters, lights, camera, normalCalcTypeBuffer);
		lightShadowShaderDispMap->render(renderer->getDeviceContext(), mountainMesh->getIndexCount());

		// Render model Mesh //
		// calculate the model world matrix
		XMMATRIX modelWorldMatrix = worldMatrix * XMMatrixTranslation(80.f, 8.f, 0.f) * XMMatrixScaling(0.5f, 0.5f, 0.5f);
		basicShaderParameters.worldMatrix = modelWorldMatrix;

		model->sendData(renderer->getDeviceContext());
		lightShadowShader->setShaderParameters(basicShaderParameters, shadowShaderParameters, lights, camera, noneNormalCalcTypeBuffer);
		lightShadowShader->render(renderer->getDeviceContext(), model->getIndexCount());
	}
	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}


// Post processing passes // 
void App1::postProcessing()
{
	// Post processing passes //
	brightPass();

	// Putting together all the textures of post-processing (in this case only glowing/bright) with the processing/production scene (post-processing texture + scene)

	// Set the main parameters to send to the shaders
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering
	XMMATRIX orthoMatrix = postProcessingSceneRenderTexture->getOrthoMatrix();  // ortho matrix for 2D rendering
	BasicShaderParameters basicShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, processingSceneRenderTexture->getShaderResourceView());

	// Set the render target to be the render to texture and clear it
	postProcessingSceneRenderTexture->setRenderTarget(renderer->getDeviceContext());
	postProcessingSceneRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 0.0f, 1.0f);
	{
		// Render
		renderer->setZBuffer(false);
		{
			orthoMeshUp->sendData(renderer->getDeviceContext());
			postProcessingShader->setShaderParameters(basicShaderParameters, glowRenderTextures.blurRenderTextures.upSampleTexture->getShaderResourceView());
			postProcessingShader->render(renderer->getDeviceContext(), orthoMeshUp->getIndexCount());
		}
		renderer->setZBuffer(true);
	}
	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}

void App1::brightPass()
{
	// Bright Pass
	// Set the render target to be the render to texture and clear it
	glowRenderTextures.brightRenderTexture->setRenderTarget(renderer->getDeviceContext());
	glowRenderTextures.brightRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 0.0f, 1.0f);
	{
		// Set the main parameters to send to the shaders
		XMMATRIX worldMatrix = renderer->getWorldMatrix();
		XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering
		XMMATRIX orthoMatrix = glowRenderTextures.brightRenderTexture->getOrthoMatrix();  // ortho matrix for 2D rendering
		BasicShaderParameters basicShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, processingSceneRenderTexture->getShaderResourceView());

		// Render
		renderer->setZBuffer(false);
		{
			orthoMeshUp->sendData(renderer->getDeviceContext());
			brightShader->setShaderParameters(basicShaderParameters);
			brightShader->render(renderer->getDeviceContext(), orthoMeshUp->getIndexCount());
		}
		renderer->setZBuffer(true);
	}
	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();

	// Blur the bright pass
	performBlurring(glowRenderTextures.brightRenderTexture, &(glowRenderTextures.blurRenderTextures), 2);
}


// Print on screen the final textures //
void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);
	{
		// RENDER THE RENDER TEXTURE SCENE
		// Requires 2D rendering and an ortho mesh.
		XMMATRIX worldMatrix = renderer->getWorldMatrix();
		XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering
		XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
		XMMATRIX firstMiniWindowsWorldMatrix = XMMatrixTranslation((3.0f * sWidth) / 4, (3.0f * sHeight) / 4, 0.0f) * XMMatrixScaling(0.5f, 0.5f, 0.5f);
		BasicShaderParameters basicShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, textureMgr->getTexture(L"brick"));

		// STARTING TO RENDER THE MINI WINDOWS (ORTHOMESHES)

		renderer->setZBuffer(false);
		{
			// Final Scene
			basicShaderParameters.texture = postProcessingSceneRenderTexture->getShaderResourceView();//processingSceneRenderTexture->getShaderResourceView(); //glowRenderTextures.brightRenderTexture->getShaderResourceView();//

			orthoMeshUp->sendData(renderer->getDeviceContext());
			textureShader->setShaderParameters(basicShaderParameters);
			textureShader->render(renderer->getDeviceContext(), orthoMeshUp->getIndexCount());

			// First Mini Window
			basicShaderParameters.worldMatrix = firstMiniWindowsWorldMatrix;
			basicShaderParameters.texture = dLightsRenderTextures[1].depthMap->getDepthMapSRV();

			orthoMeshDown->sendData(renderer->getDeviceContext());
			textureShader->setShaderParameters(basicShaderParameters);
			textureShader->render(renderer->getDeviceContext(), orthoMeshDown->getIndexCount());

			// Second Mini Window
			basicShaderParameters.worldMatrix = firstMiniWindowsWorldMatrix * XMMatrixTranslation(0.0f, -sHeight / 4.0f, 0.f);
			basicShaderParameters.texture = dLightsRenderTextures[1].blackWhiteRenderTexture->getShaderResourceView();

			orthoMeshDown->sendData(renderer->getDeviceContext());
			textureShader->setShaderParameters(basicShaderParameters);
			textureShader->render(renderer->getDeviceContext(), orthoMeshDown->getIndexCount());

			// Third Mini Window
			basicShaderParameters.worldMatrix = firstMiniWindowsWorldMatrix * XMMatrixTranslation(0.0f, -sHeight / 2.0f, 0.f);
			basicShaderParameters.texture = dLightsRenderTextures[1].blurRenderTextures.upSampleTexture->getShaderResourceView();

			orthoMeshDown->sendData(renderer->getDeviceContext());
			textureShader->setShaderParameters(basicShaderParameters);
			textureShader->render(renderer->getDeviceContext(), orthoMeshDown->getIndexCount());
		}
		renderer->setZBuffer(true);


		// GUI
		gui();

	}
	renderer->endScene();
}



// Gui function //
void App1::guiLightStats(Light* light, bool lightIsOn, std::string lightId, LightType lightType)
{
	XMFLOAT4 ambientColour, diffuseColour, specularColour;
	XMFLOAT3 position, attenuation;
	XMFLOAT2 angles;
	float specularPower, spotExponent;

	///////////////////	// For Directional Light // ////////////////////////////////////////////////////////////////////////////////
	if (lightIsOn)
	{
		if (ImGui::CollapsingHeader(("Stats - " + lightId).c_str()))
		{
			// Getting all the variables to use //
			float dAmbient[4], dDiffuse[4], dSpecular[4];
			float dAngles[3], dSpecularPower;
			float dPosition[3];

			float pAttenuation[3];

			ambientColour = light->getAmbientColour();
			dAmbient[0] = ambientColour.x;
			dAmbient[1] = ambientColour.y;
			dAmbient[2] = ambientColour.z;
			dAmbient[3] = ambientColour.w;
			diffuseColour = light->getDiffuseColour();
			dDiffuse[0] = diffuseColour.x;
			dDiffuse[1] = diffuseColour.y;
			dDiffuse[2] = diffuseColour.z;
			dDiffuse[3] = diffuseColour.w;
			specularColour = light->getSpecularColour();
			dSpecular[0] = specularColour.x;
			dSpecular[1] = specularColour.y;
			dSpecular[2] = specularColour.z;
			dSpecular[3] = specularColour.w;

			position = light->getPosition();
			dPosition[0] = position.x;
			dPosition[1] = position.y;
			dPosition[2] = position.z;

			specularPower = light->getSpecularPower();
			dSpecularPower = specularPower;

			if (lightType == LightType::kDirectional || lightType == LightType::kSpotLight)
			{
				angles = light->getAngles();
				dAngles[0] = angles.x;
				dAngles[1] = angles.y;
			}

			if (lightType == LightType::kPointLight || lightType == LightType::kSpotLight)
			{
				attenuation = light->getAttenuation();
				pAttenuation[0] = attenuation.x; // const att (we do not modify it)
				pAttenuation[1] = attenuation.y; // linear att
				pAttenuation[2] = attenuation.z; // quadratic att
			}


			// Printing all the variable on the GUI so can be modified //

			ImGui::ColorEdit4(std::string("Ambient Colour - " + lightId).c_str(), dAmbient);
			ImGui::ColorEdit4(std::string("Diffuse Colour - " + lightId).c_str(), dDiffuse);
			ImGui::ColorEdit4(std::string("Specular Colour - " + lightId).c_str(), dSpecular);
			ImGui::SliderFloat3(std::string("Position - " + lightId).c_str(), dPosition, -100.0f, 100.0f);
			ImGui::SliderFloat(std::string("Specular Power - " + lightId).c_str(), &dSpecularPower, 1.0f, 256.0f);

			if (lightType == LightType::kDirectional || lightType == LightType::kSpotLight)
			{
				ImGui::SliderAngle(std::string("Light Pitch(x) - " + lightId).c_str(), &dAngles[0]);
				ImGui::SliderAngle(std::string("Light Yaw(y) - " + lightId).c_str(), &dAngles[1]);
			}

			if (lightType == LightType::kPointLight || lightType == LightType::kSpotLight)
			{
				ImGui::SliderFloat(std::string("Linear Attenuation - " + lightId).c_str(), &pAttenuation[1], 0.0f, 1.0f);
				ImGui::SliderFloat(std::string("Quadratic Attenuation - " + lightId).c_str(), &pAttenuation[2], 0.0f, 1.0f);
			}


			// Detecting if any of the variable has been modified on the gui, if so then update in the light pointer //

			if (ambientColour.x != dAmbient[0] || ambientColour.y != dAmbient[1] || ambientColour.z != dAmbient[2]
				|| ambientColour.w != dAmbient[3])
			{
				light->setAmbientColour(dAmbient[0], dAmbient[1], dAmbient[2], dAmbient[3]);
			}
			if (diffuseColour.x != dDiffuse[0] || diffuseColour.y != dDiffuse[1] || diffuseColour.z != dDiffuse[2]
				|| diffuseColour.w != dDiffuse[3])
			{
				light->setDiffuseColour(dDiffuse[0], dDiffuse[1], dDiffuse[2], dDiffuse[3]);
			}
			if (specularColour.x != dSpecular[0] || specularColour.y != dSpecular[1] || specularColour.z != dSpecular[2]
				|| specularColour.w != dSpecular[3])
			{
				light->setSpecularColour(dSpecular[0], dSpecular[1], dSpecular[2], dSpecular[3]);
			}

			if (position.x != dPosition[0] || position.y != dPosition[1] || position.z != dPosition[2])
			{
				light->setPosition(dPosition[0], dPosition[1], dPosition[2]);
			}

			if (specularPower != dSpecularPower)
			{
				light->setSpecularPower(dSpecularPower);
			}

			if ((lightType == LightType::kDirectional || lightType == LightType::kSpotLight) && 
				(angles.x != dAngles[0] || angles.y != dAngles[1]))
			{
				light->setAngles(dAngles[0], dAngles[1]);
			}
			
			if ((lightType == LightType::kPointLight || lightType == LightType::kSpotLight) &&
				(attenuation.y != pAttenuation[1] || attenuation.z != pAttenuation[2]))
			{
				light->setAttenuation(pAttenuation[0], pAttenuation[1], pAttenuation[2]);
			}

			ImGui::Text("\n");
		}
	}
}

void App1::guiLightCheckbox(Light* light, bool* lightIsOn, std::string lightId)
{
	*lightIsOn = light->getIsOn();

	ImGui::Checkbox( ("Switch - " + lightId).c_str(), lightIsOn);

	if (light->getIsOn() != *lightIsOn)
	{
		light->setIsOn(*lightIsOn);
	}	
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI //

	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	// Camera position
	XMFLOAT3 camPos = camera->getPosition();
	ImGui::Text("Camera Pos: (%.0f, %.0f, %.0f)", camPos.x, camPos.y, camPos.z);

	// Features for displacement mapping (calculation of normals)
	if (ImGui::CollapsingHeader("Terrain - Displacement Mapping"))
	{
		ImGui::Checkbox("Calculate Normal on PS", &calculateNormalOnPS);
		ImGui::Checkbox("Calculate Avg Normal", &calculateAvgNormal);
		ImGui::Checkbox("Render Normal Colour", &renderNormalColour);
	}

	// For lights
	if (ImGui::CollapsingHeader("Lights"))
	{
		// Turn on/off lights //

		bool dLightIsTurnedOn[kDirectionalLightNum], pLightIsTurnedOn[kPointLightNum], sLightIsTurnedOn[kSpotLightNum];
		std::string dLightId[kDirectionalLightNum], pLightId[kPointLightNum], sLightId[kSpotLightNum];

		ImGui::Text("Lights switches:");

		for (int i = 0; i < kDirectionalLightNum; i++)
		{
			dLightId[i] = "DL" + std::to_string(i);

			guiLightCheckbox(lights.dLights[i], &dLightIsTurnedOn[i], dLightId[i]);
		}

		for (int i = 0; i < kPointLightNum; i++)
		{
			pLightId[i] = "PL" + std::to_string(i);

			guiLightCheckbox(lights.pLights[i], &pLightIsTurnedOn[i], pLightId[i]);
		}

		for (int i = 0; i < kSpotLightNum; i++)
		{
			sLightId[i] = "SL" + std::to_string(i);

			guiLightCheckbox(lights.sLights[i], &sLightIsTurnedOn[i], sLightId[i]);
		}

		// Update the stats of the lights //

		ImGui::Text("Lights Stats:");

		for (int i = 0; i < kDirectionalLightNum; i++)
		{
			guiLightStats(lights.dLights[i], dLightIsTurnedOn[i], dLightId[i], LightType::kDirectional);
		}

		for (int i = 0; i < kPointLightNum; i++)
		{
			guiLightStats(lights.pLights[i], pLightIsTurnedOn[i], pLightId[i], LightType::kPointLight);
		}

		for (int i = 0; i < kSpotLightNum; i++)
		{
			guiLightStats(lights.sLights[i], sLightIsTurnedOn[i], sLightId[i], LightType::kSpotLight);
		}
	}

	////////////////////////////////

	ImGui::Text("\nPress WASDQE to move the camera along axis \nUse the arrow keys or the mouse for turning the camera.");

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
