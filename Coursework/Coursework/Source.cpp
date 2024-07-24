// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"
#include "Utils.h"
#include "BasicShaderParameters.h"

App1::App1()
{
	// Shaders
	lightShadowShader = nullptr;

	// objects
	terrainMesh = nullptr;

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

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	initMeshes();
	initLights();
	initShaders();

	// load texture
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");

	// initialise camera
	camera->setPosition(-2.0f, 20.0f, -30.0f);

	origin = lights.pLights[0]->getPosition();
}


void App1::initLights()
{
	// Initialise lights //
	lights.dLights[0] = new Light();
	lights.dLights[0]->setAmbientColour(0.1f, 0.1f, 0.1f, 1.0f); // it should be a range value [0.1, 0.3]
	lights.dLights[0]->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f); // white
	lights.dLights[0]->setSpecularColour(1.0f, 1.0f, 1.0f, 1.0f);
	lights.dLights[0]->setDirection(0.0f, -0.7f, -0.7f); // direction down onto the geometry
	lights.dLights[0]->setPosition(0.f, 0.f, 100.f);
	lights.dLights[0]->setSpecularPower(256.0f);
	lights.dLights[0]->setIsOn(true);

	lights.dLights[1] = new Light();
	lights.dLights[1]->setAmbientColour(0.0f, 0.0f, 0.0f, 1.0f); // it should be a range value [0.1, 0.3]
	lights.dLights[1]->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f); // white
	lights.dLights[1]->setSpecularColour(1.0f, 1.0f, 1.0f, 1.0f);
	lights.dLights[1]->setAngles(Utils::degreesToRads(-138.0f), Utils::degreesToRads(90.0f));
	lights.dLights[1]->setPosition(0.f, 0.f, 100.f);
	lights.dLights[1]->setSpecularPower(256.0f);
	lights.dLights[1]->setIsOn(true);

	lights.pLights[0] = new Light();
	lights.pLights[0]->setAmbientColour(0.1f, 0.1f, 0.1f, 1.0f); // it should be a range value [0.1, 0.3]
	lights.pLights[0]->setDiffuseColour(1.0f, 0.0f, 0.0f, 1.0f); // red
	lights.pLights[0]->setSpecularColour(1.0f, 0.0f, 0.0f, 1.0f);
	lights.pLights[0]->setPosition(0.0f, 14.0f, 25.0f);
	lights.pLights[0]->setSpecularPower(128.0f);
	lights.pLights[0]->setAttenuation(1.0f, 0.0f, 0.0f);

	lights.sLights[0] = new Light();
	lights.sLights[0]->setAmbientColour(0.1f, 0.1f, 0.1f, 1.0f); // it should be a range value [0.2, 0.3]
	lights.sLights[0]->setDiffuseColour(0.0f, 0.0f, 1.0f, 1.0f); // blue
	lights.sLights[0]->setSpecularColour(0.0f, 0.0f, 1.0f, 1.0f);
	lights.sLights[0]->setPosition(0.0f, 4.0f, 10.0f); // as it is a lights.sLights it has a position
	lights.sLights[0]->setSpecularPower(256.0f);
	lights.sLights[0]->setDirection(0.0f, -1.0f, 0.0f); // direction down onto the geometry
	lights.sLights[0]->setSpotExponent(5.0f);
	lights.sLights[0]->setAttenuation(1.0f, 0.0f, 0.0f);

	// Initialise renderer textures
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
}

void App1::initMeshes()
{
	// Create terrain:
	terrainMesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
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
	lightShadowShader = new LightShadowShader(renderer->getDevice(), wnd);
	bwShadowShader = new BWShadowShader(renderer->getDevice(), wnd);
	textureShader = new TextureShader(renderer->getDevice(), wnd);
	disMappingShader = new DisplacementMappingShader(renderer->getDevice(), wnd);
}

void App1::initShadowLightRenderTextures(ShadowLightRenderTextures* shadowLightsRenderTextures)
{
	const int HALFSCREEN_WITH = sWidth / 2;
	const int HALFSCREEN_HEIGHT = sHeight / 2;
	//shadowLightsRenderTextures->renderTexture = new RenderTexture(renderer->getDevice(), (float)SHADOWMAP_WIDTH, (float)SHADOWMAP_HEIGHT, SCREEN_NEAR, SCREEN_DEPTH);
	shadowLightsRenderTextures->shadowMap = new ShadowMap(renderer->getDevice(), (float)SHADOWMAP_WIDTH, (float)SHADOWMAP_HEIGHT);
	shadowLightsRenderTextures->blackWhiteRenderTexture = new RenderTexture(renderer->getDevice(), sWidth, sHeight, SCREEN_NEAR, SCREEN_DEPTH);
	shadowLightsRenderTextures->downSampleTexture = new RenderTexture(renderer->getDevice(), HALFSCREEN_WITH, HALFSCREEN_HEIGHT, SCREEN_NEAR, SCREEN_DEPTH);
	shadowLightsRenderTextures->horizontalBlurTexture = new RenderTexture(renderer->getDevice(), HALFSCREEN_WITH, HALFSCREEN_HEIGHT, SCREEN_NEAR, SCREEN_DEPTH);
	shadowLightsRenderTextures->verticalBlurTexture = new RenderTexture(renderer->getDevice(), HALFSCREEN_WITH, HALFSCREEN_HEIGHT, SCREEN_NEAR, SCREEN_DEPTH);
	shadowLightsRenderTextures->upSampleTexture = new RenderTexture(renderer->getDevice(), sWidth, sHeight, SCREEN_NEAR, SCREEN_DEPTH);
}


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

	// update the origin of the circle that draw the spheres
	// base on the point light
	//origin = lights.pLights[0]->getPosition();

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


	/*
	////////////////////// Move directional light
	// bool result;
	//static float lightAngle = 270.0f;
	float radians;
	float sunSpeed = 200.0f;
	//static float lightPosX = 9.0f;

	// Update the position of the light each frame.
	lightPosX -= 0.003f * dt * sunSpeed;

	// Update the angle of the light each frame.
	lightAngle -= 0.03f * dt * sunSpeed;

	if (lightAngle < 100.0f)
	{
		lightAngle = 240.0f;

		// Reset the light position also.
		lightPosX = 9.0f;
	}
	radians = lightAngle * 0.0174532925f;

	// Update the direction of the light.
	lights.dLights[0]->setDirection(sinf(radians), cosf(radians), 0.0f);
	lights.dLights[0]->setPosition(lightPosX, 60.0f, 40.0f);
	lights.dLights[0]->setLookAt(-lightPosX, 0.0f, 0.0f);
	*/

	///////////////

	return true;
}

bool App1::render()
{
	// First render the scene to a texture (depth map of all the lights)
	performLightPasses(depthPass);

	// Next Transform that depth map textures to black and white textures
	performLightPasses(bwPass);

	// Blur all the black and white shadow textures
	// (Down sample, horizontal and vertical blur, and up sample)
	blurBWShadowTextures();
	//applyAntialiasingBWShadowTextures();

	// Render scene using the soft shadows
	finalPass();

	return true;
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
	for (int i = 0; i < kPointLightNum; i++)
	{
		pointLightPass(pLightsRenderTextures, lights.pLights[i], func);
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
	XMMATRIX lightProjectionMatrix = light->getProjectionMatrix();

	func(renderTextureShadowLights, light->getIsOn(), lightViewMatrix, lightProjectionMatrix);
}


void App1::lightDepthPass(ShadowLightRenderTextures* renderTextureShadowLights, bool lightIsOn, XMMATRIX lightViewMatrix, XMMATRIX lightProjectionMatrix)
{
	// Set the render target to be the render to texture and clear it
	//renderTextureShadowLights->renderTexture->setRenderTarget(renderer->getDeviceContext());
	//renderTextureShadowLights->renderTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 0.0f, 0.0f, 1.0f);
	renderTextureShadowLights->shadowMap->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	BasicShaderParameters basicShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);

	// Render spheres from light perspective
	for (std::pair<SphereMesh*, float*> sphere : sphereMeshes)
	{
		XMFLOAT3 newPos = getCircunferencePoint(origin, radius, *sphere.second);
		XMMATRIX sphereWorldMatrix = worldMatrix * XMMatrixTranslation(newPos.x, newPos.y, newPos.z);
		basicShaderParameters.worldMatrix = sphereWorldMatrix;

		sphere.first->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(basicShaderParameters);
		depthShader->render(renderer->getDeviceContext(), sphere.first->getIndexCount());
	}

	// Render floor from light perspective
	XMMATRIX floorWorldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	basicShaderParameters.worldMatrix = floorWorldMatrix;

	terrainMesh->sendData(renderer->getDeviceContext());
	depthShader->setShaderParameters(basicShaderParameters);
	depthShader->render(renderer->getDeviceContext(), terrainMesh->getIndexCount());


	// Render model
	XMMATRIX modelWorldMatrix = worldMatrix * XMMatrixTranslation(0.f, 7.f, 5.f) * XMMatrixScaling(0.5f, 0.5f, 0.5f);
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
	ID3D11ShaderResourceView* lightShadowDepthMap = renderTextureShadowLights->shadowMap->getDepthMapSRV();
	//ID3D11ShaderResourceView* lightShadowDepthMap = renderTextureShadowLights->renderTexture->getShaderResourceView();
	BasicShaderParameters basicShaderParameters(renderer->getDeviceContext(), worldMatrix, camViewMatrix, projectionMatrix, lightShadowDepthMap);

	// Set the render target to be the render to texture and clear it
	renderTextureShadowLights->blackWhiteRenderTexture->setRenderTarget(renderer->getDeviceContext());
	renderTextureShadowLights->blackWhiteRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 1.0f, 0.0f, 1.0f);

	// Render spheres from light perspective
	for (std::pair<SphereMesh*, float*> sphere : sphereMeshes)
	{
		XMFLOAT3 newPos = getCircunferencePoint(origin, radius, *sphere.second);
		XMMATRIX sphereWorldMatrix = worldMatrix * XMMatrixTranslation(newPos.x, newPos.y, newPos.z);
		basicShaderParameters.worldMatrix = sphereWorldMatrix;

		sphere.first->sendData(renderer->getDeviceContext());
		bwShadowShader->setShaderParameters(basicShaderParameters, lightViewMatrix, lightProjectionMatrix, lightIsOn);
		bwShadowShader->render(renderer->getDeviceContext(), sphere.first->getIndexCount());
	}

	// Render floor
	XMMATRIX floorWorldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
	basicShaderParameters.worldMatrix = floorWorldMatrix;

	terrainMesh->sendData(renderer->getDeviceContext());
	bwShadowShader->setShaderParameters(basicShaderParameters, lightViewMatrix, lightProjectionMatrix, lightIsOn);
	bwShadowShader->render(renderer->getDeviceContext(), terrainMesh->getIndexCount());

	// Render model
	XMMATRIX modelWorldMatrix = worldMatrix * XMMatrixTranslation(0.f, 7.f, 5.f) * XMMatrixScaling(0.5f, 0.5f, 0.5f);
	basicShaderParameters.worldMatrix = modelWorldMatrix;

	model->sendData(renderer->getDeviceContext());
	bwShadowShader->setShaderParameters(basicShaderParameters, lightViewMatrix, lightProjectionMatrix, lightIsOn);
	bwShadowShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}


void App1::blurBWShadowTextures()
{

	for (int i = 0; i < kDirectionalLightNum; i++)
	{
		// Down sample the black and white scene textures.
		renderToTexturePass(dLightsRenderTextures[i].blackWhiteRenderTexture, dLightsRenderTextures[i].downSampleTexture, orthoMeshDown, BlurType::None);
		// Perform horizontal blur on the down sampled textures
		renderToTexturePass(dLightsRenderTextures[i].downSampleTexture, dLightsRenderTextures[i].horizontalBlurTexture, orthoMeshDown, BlurType::Horizontal);
		// Perform vertical blur on the down sampled textures
		renderToTexturePass(dLightsRenderTextures[i].horizontalBlurTexture, dLightsRenderTextures[i].verticalBlurTexture, orthoMeshDown, BlurType::Vertical);
		// Finally up sample the final blurred render to textures that can now be used in the light-shadow shader.
		renderToTexturePass(dLightsRenderTextures[i].verticalBlurTexture, dLightsRenderTextures[i].upSampleTexture, orthoMeshUp, BlurType::None);
	}

	for (int i = 0; i < kSpotLightNum; i++)
	{
		// Down sample the black and white scene textures.
		renderToTexturePass(sLightsRenderTextures[i].blackWhiteRenderTexture, sLightsRenderTextures[i].downSampleTexture, orthoMeshDown, BlurType::None);
		// Perform horizontal blur on the down sampled textures
		renderToTexturePass(sLightsRenderTextures[i].downSampleTexture, sLightsRenderTextures[i].horizontalBlurTexture, orthoMeshDown, BlurType::Horizontal);
		// Perform vertical blur on the down sampled textures
		renderToTexturePass(sLightsRenderTextures[i].horizontalBlurTexture, sLightsRenderTextures[i].verticalBlurTexture, orthoMeshDown, BlurType::Vertical);
		// Finally up sample the final blurred render to textures that can now be used in the light-shadow shader.
		renderToTexturePass(sLightsRenderTextures[i].verticalBlurTexture, sLightsRenderTextures[i].upSampleTexture, orthoMeshUp, BlurType::None);
	}

	for (int lightId = 0; lightId < kPointLightNum; lightId++)
	{
		for (int faceId = 0; faceId < kPointLightFacesNum; faceId++)
		{
			// Down sample the black and white scene textures.
			renderToTexturePass(sLightsRenderTextures[faceId + (lightId * kPointLightFacesNum)].blackWhiteRenderTexture, sLightsRenderTextures[faceId + (lightId * kPointLightFacesNum)].downSampleTexture, orthoMeshDown, BlurType::None);
			// Perform horizontal blur on the down sampled textures
			renderToTexturePass(sLightsRenderTextures[faceId + (lightId * kPointLightFacesNum)].downSampleTexture, sLightsRenderTextures[faceId + (lightId * kPointLightFacesNum)].horizontalBlurTexture, orthoMeshDown, BlurType::Horizontal);
			// Perform vertical blur on the down sampled textures
			renderToTexturePass(sLightsRenderTextures[faceId + (lightId * kPointLightFacesNum)].horizontalBlurTexture, sLightsRenderTextures[faceId + (lightId * kPointLightFacesNum)].verticalBlurTexture, orthoMeshDown, BlurType::Vertical);
			// Finally up sample the final blurred render to textures that can now be used in the light-shadow shader.
			renderToTexturePass(sLightsRenderTextures[faceId + (lightId * kPointLightFacesNum)].verticalBlurTexture, sLightsRenderTextures[faceId + (lightId * kPointLightFacesNum)].upSampleTexture, orthoMeshUp, BlurType::None);
		}
	}
}

void App1::renderToTexturePass(RenderTexture* fromRenderTexture, RenderTexture* toRenderTexture, OrthoMesh* orthoMesh, BlurType blurType)
{
	// Set the render target to be the render to texture and clear it
	toRenderTexture->setRenderTarget(renderer->getDeviceContext());
	toRenderTexture->clearRenderTarget(renderer->getDeviceContext(), 1.0f, 1.0f, 0.0f, 1.0f);

	// Set the main parameters to send to the shaders
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering
	XMMATRIX orthoMatrix = toRenderTexture->getOrthoMatrix();  // ortho matrix for 2D rendering
	BasicShaderParameters basicShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, fromRenderTexture->getShaderResourceView());

	// Render for Horizontal Blur
	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(basicShaderParameters, blurType);
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
}


void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();
	// Set the main parameters to send to the shaders
	BasicShaderParameters basicShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"));

	ShadowShaderParameters shadowShaderParameters;
	for (int i = 0; i < kDirectionalLightNum; i++)
	{
		shadowShaderParameters.dLightShadowTextures[i] = dLightsRenderTextures[i].upSampleTexture->getShaderResourceView();
	}
	for (int i = 0; i < kSpotLightNum; i++)
	{
		shadowShaderParameters.sLightShadowTextures[i] = sLightsRenderTextures[i].upSampleTexture->getShaderResourceView();
	}
	for (int lightId = 0; lightId < kPointLightNum; lightId++)
	{
		for (int faceId = 0; faceId < kPointLightFacesNum; faceId++)
		{
			shadowShaderParameters.pLightShadowTextures[faceId + (lightId * kPointLightFacesNum)] = pLightsRenderTextures[faceId + (lightId * kPointLightFacesNum)].upSampleTexture->getShaderResourceView();
		}
	}

	shadowShaderParameters.shadowFar = SCREEN_DEPTH;
	shadowShaderParameters.shadowNear = SCREEN_NEAR;


	// Render spheres //
	for (std::pair<SphereMesh*, float*> sphere : sphereMeshes)
	{
		// calculate world position for this sphere mesh
		XMFLOAT3 newPos = getCircunferencePoint(origin, radius, *sphere.second);
		XMMATRIX sMWorldMatrix = worldMatrix * XMMatrixTranslation(newPos.x, newPos.y, newPos.z);
		basicShaderParameters.worldMatrix = sMWorldMatrix;

		sphere.first->sendData(renderer->getDeviceContext());
		lightShadowShader->setShaderParameters(basicShaderParameters, shadowShaderParameters, lights, camera);
		lightShadowShader->render(renderer->getDeviceContext(), sphere.first->getIndexCount());
	}

	// Point Light mesh //
	if (lights.pLights[0]->getIsOn() == true)
	{
		// calculate world position for this point light mesh
		XMFLOAT2 pointLightAngles = lights.pLights[0]->getAngles();
		XMFLOAT3 pointLightPos = lights.pLights[0]->getPosition();
		XMMATRIX pLWorldMatrix = worldMatrix * XMMatrixRotationRollPitchYaw(pointLightAngles.x, pointLightAngles.y, 1.0f) * XMMatrixScaling(0.5f, 0.5f, 0.5f) * XMMatrixTranslation(pointLightPos.x, pointLightPos.y, pointLightPos.z);
		basicShaderParameters.worldMatrix = pLWorldMatrix;

		pLightMesh->sendData(renderer->getDeviceContext());
		textureShader->setShaderParameters(basicShaderParameters);
		textureShader->render(renderer->getDeviceContext(), pLightMesh->getIndexCount());
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

	lightShadowShader->setShaderParameters(basicShaderParameters, shadowShaderParameters, lights, camera);
	lightShadowShader->render(renderer->getDeviceContext(), terrainMesh->getIndexCount());

	// Render model Mesh //
	// calculate the model world matrix
	XMMATRIX modelWorldMatrix = worldMatrix * XMMatrixTranslation(0.f, 7.f, 5.f) * XMMatrixScaling(0.5f, 0.5f, 0.5f);
	basicShaderParameters.worldMatrix = modelWorldMatrix;

	model->sendData(renderer->getDeviceContext());
	lightShadowShader->setShaderParameters(basicShaderParameters, shadowShaderParameters, lights, camera);
	lightShadowShader->render(renderer->getDeviceContext(), model->getIndexCount());

	// RENDER THE RENDER TEXTURE SCENE
	// Requires 2D rendering and an ortho mesh.
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();  // ortho matrix for 2D rendering
	XMMATRIX firstMiniWindowsWorldMatrix = XMMatrixTranslation((3.0f * sWidth) / 4, (3.0f * sHeight) / 4, 0.0f) * XMMatrixScaling(0.5f, 0.5f, 0.5f);
	basicShaderParameters.viewMatrix = orthoViewMatrix;
	basicShaderParameters.projectionMatrix = orthoMatrix;


	// STARTING TO RENDER THE MINI WINDOWS (ORTHOMESHES)

	renderer->setZBuffer(false);

	// First Mini Window
	basicShaderParameters.worldMatrix = firstMiniWindowsWorldMatrix;
	basicShaderParameters.texture = dLightsRenderTextures[0].shadowMap->getDepthMapSRV();

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
	basicShaderParameters.texture = dLightsRenderTextures[1].upSampleTexture->getShaderResourceView();

	orthoMeshDown->sendData(renderer->getDeviceContext());
	textureShader->setShaderParameters(basicShaderParameters);
	textureShader->render(renderer->getDeviceContext(), orthoMeshDown->getIndexCount());

	renderer->setZBuffer(true);


	// GUI
	gui();

	renderer->endScene();
}


XMFLOAT3 App1::getCircunferencePoint(XMFLOAT3 origin, float radius, float angle)
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

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	XMFLOAT3 camPos = camera->getPosition();
	ImGui::Text("Camera Pos: (%.0f, %.0f, %.0f)", camPos.x, camPos.y, camPos.z);


	if (ImGui::CollapsingHeader("Lights"))
	{
		// Turn on/off lights
		bool dLightIsTurnedOn, pLightIsTurnedOn, sLightIsTurnedOn;
		dLightIsTurnedOn = lights.dLights[0]->getIsOn();
		pLightIsTurnedOn = lights.pLights[0]->getIsOn();
		sLightIsTurnedOn = lights.sLights[0]->getIsOn();

		ImGui::Checkbox("Directional Light", &dLightIsTurnedOn);
		ImGui::Checkbox("Point Light", &pLightIsTurnedOn);
		ImGui::Checkbox("Spot Light", &sLightIsTurnedOn);

		if (lights.dLights[0]->getIsOn() != dLightIsTurnedOn)
		{
			lights.dLights[0]->setIsOn(dLightIsTurnedOn);
		}
		if (lights.pLights[0]->getIsOn() != pLightIsTurnedOn)
		{
			lights.pLights[0]->setIsOn(pLightIsTurnedOn);
		}
		if (lights.sLights[0]->getIsOn() != sLightIsTurnedOn)
		{
			lights.sLights[0]->setIsOn(sLightIsTurnedOn);
		}

		XMFLOAT4 ambientColour, diffuseColour, specularColour;
		XMFLOAT3 position, attenuation;
		XMFLOAT2 angles;
		float specularPower, spotExponent;

		///////////////////	// For Directional Light // ////////////////////////////////////////////////////////////////////////////////
		if (dLightIsTurnedOn)
		{
			if (ImGui::CollapsingHeader("DirectionalLight"))
			{
				float dAmbient[4], dDiffuse[4], dSpecular[4];
				float dAngles[3], dSpecularPower;
				float dPosition[3];

				ambientColour = lights.dLights[0]->getAmbientColour();
				dAmbient[0] = ambientColour.x;
				dAmbient[1] = ambientColour.y;
				dAmbient[2] = ambientColour.z;
				dAmbient[3] = ambientColour.w;
				diffuseColour = lights.dLights[0]->getDiffuseColour();
				dDiffuse[0] = diffuseColour.x;
				dDiffuse[1] = diffuseColour.y;
				dDiffuse[2] = diffuseColour.z;
				dDiffuse[3] = diffuseColour.w;
				specularColour = lights.dLights[0]->getSpecularColour();
				dSpecular[0] = specularColour.x;
				dSpecular[1] = specularColour.y;
				dSpecular[2] = specularColour.z;
				dSpecular[3] = specularColour.w;

				position = lights.dLights[0]->getPosition();
				dPosition[0] = position.x;
				dPosition[1] = position.y;
				dPosition[2] = position.z;

				angles = lights.dLights[0]->getAngles();
				dAngles[0] = angles.x;
				dAngles[1] = angles.y;
				specularPower = lights.dLights[0]->getSpecularPower();
				dSpecularPower = specularPower;

				ImGui::ColorEdit4("Ambient Colour - DL", dAmbient);
				ImGui::ColorEdit4("Diffuse Colour - DL", dDiffuse);
				ImGui::ColorEdit4("Specular Colour - DL", dSpecular);
				ImGui::SliderFloat3("Light Position - DL", dPosition, -100.0f, 100.0f);


				ImGui::SliderAngle("Light Pitch(x) - DL", &dAngles[0]);
				ImGui::SliderAngle("Light Yaw(y) - DL", &dAngles[1]);

				ImGui::SliderFloat("Specular Power - DL", &dSpecularPower, 1.0f, 256.0f);

				if (ambientColour.x != dAmbient[0] || ambientColour.y != dAmbient[1] || ambientColour.z != dAmbient[2]
					|| ambientColour.w != dAmbient[3])
				{
					lights.dLights[0]->setAmbientColour(dAmbient[0], dAmbient[1], dAmbient[2], dAmbient[3]);
				}
				if (diffuseColour.x != dDiffuse[0] || diffuseColour.y != dDiffuse[1] || diffuseColour.z != dDiffuse[2]
					|| diffuseColour.w != dDiffuse[3])
				{
					lights.dLights[0]->setDiffuseColour(dDiffuse[0], dDiffuse[1], dDiffuse[2], dDiffuse[3]);
				}
				if (specularColour.x != dSpecular[0] || specularColour.y != dSpecular[1] || specularColour.z != dSpecular[2]
					|| specularColour.w != dSpecular[3])
				{
					lights.dLights[0]->setSpecularColour(dSpecular[0], dSpecular[1], dSpecular[2], dSpecular[3]);
				}

				if (position.x != dPosition[0] || position.y != dPosition[1] || position.z != dPosition[2])
				{
					lights.dLights[0]->setPosition(dPosition[0], dPosition[1], dPosition[2]);
				}

				if (angles.x != dAngles[0] || angles.y != dAngles[1])
				{
					lights.dLights[0]->setAngles(dAngles[0], dAngles[1]);
				}
				if (specularPower != dSpecularPower)
				{
					lights.dLights[0]->setSpecularPower(dSpecularPower);
				}
				ImGui::Text("\n");
			}
		}

		///////////////////	// For Point Light // ///////////////////////////////////////////////////////////////////////////////////////////////
		if (pLightIsTurnedOn)
		{
			if (ImGui::CollapsingHeader("PointLight"))
			{
				float pAmbient[4], pDiffuse[4], pSpecular[4];
				float pPosition[3], pSpecularPower;
				float pAttenuation[3];

				ambientColour = lights.pLights[0]->getAmbientColour();
				pAmbient[0] = ambientColour.x;
				pAmbient[1] = ambientColour.y;
				pAmbient[2] = ambientColour.z;
				pAmbient[3] = ambientColour.w;
				diffuseColour = lights.pLights[0]->getDiffuseColour();
				pDiffuse[0] = diffuseColour.x;
				pDiffuse[1] = diffuseColour.y;
				pDiffuse[2] = diffuseColour.z;
				pDiffuse[3] = diffuseColour.w;
				specularColour = lights.pLights[0]->getSpecularColour();
				pSpecular[0] = specularColour.x;
				pSpecular[1] = specularColour.y;
				pSpecular[2] = specularColour.z;
				pSpecular[3] = specularColour.w;

				position = lights.pLights[0]->getPosition();
				pPosition[0] = position.x;
				pPosition[1] = position.y;
				pPosition[2] = position.z;
				specularPower = lights.pLights[0]->getSpecularPower();
				pSpecularPower = specularPower;

				attenuation = lights.pLights[0]->getAttenuation();
				pAttenuation[0] = attenuation.x; // const att (we do not modify it)
				pAttenuation[1] = attenuation.y; // linear att
				pAttenuation[2] = attenuation.z; // quadratic att

				ImGui::ColorEdit4("Ambient Colour - PL", pAmbient);
				ImGui::ColorEdit4("Diffuse Colour - PL", pDiffuse);
				ImGui::ColorEdit4("Specular Colour - PL", pSpecular);

				ImGui::SliderFloat3("Light Position - PL", pPosition, -100.0f, 100.0f);
				ImGui::SliderFloat("Specular Power - PL", &pSpecularPower, 1.0f, 256.0f);

				ImGui::SliderFloat("Linear Attenuation - PL", &pAttenuation[1], 0.0f, 4.0f);
				ImGui::SliderFloat("Quadratic Attenuation - PL", &pAttenuation[2], 0.0f, 4.0f);

				if (ambientColour.x != pAmbient[0] || ambientColour.y != pAmbient[1] || ambientColour.z != pAmbient[2]
					|| ambientColour.w != pAmbient[3])
				{
					lights.pLights[0]->setAmbientColour(pAmbient[0], pAmbient[1], pAmbient[2], pAmbient[3]);
				}
				if (diffuseColour.x != pDiffuse[0] || diffuseColour.y != pDiffuse[1] || diffuseColour.z != pDiffuse[2]
					|| diffuseColour.w != pDiffuse[3])
				{
					lights.pLights[0]->setDiffuseColour(pDiffuse[0], pDiffuse[1], pDiffuse[2], pDiffuse[3]);
				}
				if (specularColour.x != pSpecular[0] || specularColour.y != pSpecular[1] || specularColour.z != pSpecular[2]
					|| specularColour.w != pSpecular[3])
				{
					lights.pLights[0]->setSpecularColour(pSpecular[0], pSpecular[1], pSpecular[2], pSpecular[3]);
				}

				if (position.x != pPosition[0] || position.y != pPosition[1] || position.z != pPosition[2])
				{
					lights.pLights[0]->setPosition(pPosition[0], pPosition[1], pPosition[2]);
				}
				if (specularPower != pSpecularPower)
				{
					lights.pLights[0]->setSpecularPower(pSpecularPower);
				}

				if (attenuation.y != pAttenuation[1] || attenuation.z != pAttenuation[2])
				{
					lights.pLights[0]->setAttenuation(pAttenuation[0], pAttenuation[1], pAttenuation[2]);
				}
				ImGui::Text("\n");
			}
		}

		///////////////////	// For SpotLight // /////////////////////////////////////////////////////////////////////////////////////////////////
		if (sLightIsTurnedOn)
		{
			if (ImGui::CollapsingHeader("SpotLight"))
			{
				float sAmbient[4], sDiffuse[4], sSpecular[4];
				float sPosition[3], sSpecularPower;
				float sAngles[3], sSpotExponent;
				float sAttenuation[3];

				ambientColour = lights.sLights[0]->getAmbientColour();
				sAmbient[0] = ambientColour.x;
				sAmbient[1] = ambientColour.y;
				sAmbient[2] = ambientColour.z;
				sAmbient[3] = ambientColour.w;
				diffuseColour = lights.sLights[0]->getDiffuseColour();
				sDiffuse[0] = diffuseColour.x;
				sDiffuse[1] = diffuseColour.y;
				sDiffuse[2] = diffuseColour.z;
				sDiffuse[3] = diffuseColour.w;
				specularColour = lights.sLights[0]->getSpecularColour();
				sSpecular[0] = specularColour.x;
				sSpecular[1] = specularColour.y;
				sSpecular[2] = specularColour.z;
				sSpecular[3] = specularColour.w;

				position = lights.sLights[0]->getPosition();
				sPosition[0] = position.x;
				sPosition[1] = position.y;
				sPosition[2] = position.z;
				specularPower = lights.sLights[0]->getSpecularPower();
				sSpecularPower = specularPower;

				angles = lights.sLights[0]->getAngles();
				sAngles[0] = angles.x;
				sAngles[1] = angles.y;
				spotExponent = lights.sLights[0]->getSpotExponent();
				sSpotExponent = spotExponent;

				attenuation = lights.sLights[0]->getAttenuation();
				sAttenuation[0] = attenuation.x; // const att (we do not modify it)
				sAttenuation[1] = attenuation.y; // linear att
				sAttenuation[2] = attenuation.z; // quadratic att


				ImGui::ColorEdit4("Ambient Colour - SL", sAmbient);
				ImGui::ColorEdit4("Diffuse Colour - SL", sDiffuse);
				ImGui::ColorEdit4("Specular Colour - SL", sSpecular);

				ImGui::SliderFloat3("Light Position - SL", sPosition, -100.0f, 100.0f);
				ImGui::SliderFloat("Specular Power - SL", &sSpecularPower, 1.0f, 256.0f);

				ImGui::SliderAngle("Light Pitch(x) - SL", &sAngles[0]);
				ImGui::SliderAngle("Light Yaw(y) - SL", &sAngles[1]);
				ImGui::SliderFloat("SpotExponent - SL", &sSpotExponent, 1.0f, 50.0f);

				ImGui::SliderFloat("Linear Attenuation - SL", &sAttenuation[1], 0.0f, 4.0f);
				ImGui::SliderFloat("Quadratic Attenuation - SL", &sAttenuation[2], 0.0f, 4.0f);


				if (ambientColour.x != sAmbient[0] || ambientColour.y != sAmbient[1] || ambientColour.z != sAmbient[2]
					|| ambientColour.w != sAmbient[3])
				{
					lights.sLights[0]->setAmbientColour(sAmbient[0], sAmbient[1], sAmbient[2], sAmbient[3]);
				}
				if (diffuseColour.x != sDiffuse[0] || diffuseColour.y != sDiffuse[1] || diffuseColour.z != sDiffuse[2]
					|| diffuseColour.w != sDiffuse[3])
				{
					lights.sLights[0]->setDiffuseColour(sDiffuse[0], sDiffuse[1], sDiffuse[2], sDiffuse[3]);
				}
				if (specularColour.x != sSpecular[0] || specularColour.y != sSpecular[1] || specularColour.z != sSpecular[2]
					|| specularColour.w != sSpecular[3])
				{
					lights.sLights[0]->setSpecularColour(sSpecular[0], sSpecular[1], sSpecular[2], sSpecular[3]);
				}

				if (position.x != sPosition[0] || position.y != sPosition[1] || position.z != sPosition[2])
				{
					lights.sLights[0]->setPosition(sPosition[0], sPosition[1], sPosition[2]);
				}
				if (specularPower != sSpecularPower)
				{
					lights.sLights[0]->setSpecularPower(sSpecularPower);
				}

				if (angles.x != sAngles[0] || angles.y != sAngles[1])
				{
					lights.sLights[0]->setAngles(sAngles[0], sAngles[1]);
				}
				if (spotExponent != sSpotExponent)
				{
					lights.sLights[0]->setSpotExponent(sSpotExponent);
				}

				if (attenuation.y != sAttenuation[1] || attenuation.z != sAttenuation[2])
				{
					lights.sLights[0]->setAttenuation(sAttenuation[0], sAttenuation[1], sAttenuation[2]);
				}
				ImGui::Text("\n");
			}
		}
	}

	////////////////////////////////

	ImGui::Text("\nPress WASDQE to move the camera along axis \nUse the arrow keys for turning the camera.");



	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
