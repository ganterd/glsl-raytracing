#include "scene.hpp"

namespace SGE
{
	Scene::Scene()
	{
		mCamera = new Camera();
		ShaderManager::init();
		ShaderManager::loadShader("raytracing/geometry_pass");
		ShaderManager::loadShader("raytracing/raytrace_pass");
		ShaderManager::loadShader("raytracing/blit");
		Time::init();
		overlayQuad = new OverlayQuad();

		int bufferWidth = DisplayManager::getDisplayInstance()->size().width;
		int bufferHeight = DisplayManager::getDisplayInstance()->size().height;
		renderTarget = new GLSLRenderTarget(bufferWidth, bufferHeight);
		renderTarget->addRenderBuffer(IRenderBuffer::BufferType::Position, ITexture::DataType::Float); // Position g-buffer
		renderTarget->addRenderBuffer(IRenderBuffer::BufferType::Depth, ITexture::DataType::Float);

		cachedFrame = new GLSLRenderTarget(bufferWidth, bufferHeight);
		cachedFrame->addRenderBuffer(IRenderBuffer::BufferType::Color, ITexture::DataType::Float);
		cachedFrame->addRenderBuffer(IRenderBuffer::BufferType::Depth, ITexture::DataType::Float);
		cachedFrame->clear();

		currentIteration = 0;
		iterationsPerPatch = 1000;
		patchSizeX = 1.0f;
		patchSizeY = 1.0f / (float)(bufferHeight / 16);
		patchX = 0.0f;
		patchY = 0.0f;

		mRootEntity = new Entity();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Scene::addEntity(Entity* entity)
	{
		mRootEntity->addChild(entity);
		if(entity->mCamera)
			mCamera = entity->mCamera;
	}

	void Scene::update()
	{
		Time::tick();
		//Input::update();
		mCamera->update();

		mRootEntity->update();
		if(mBVH.mTargetSceneRoot != mRootEntity)
		{
			mBVH.construct(mRootEntity);
			mBVHSSBO.toSSBO(&mBVH);
			mBVHSSBO.bind(11, 10);

			mSceneLights = extractLights();
		}
	}

	void Scene::draw()
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		IShader* shader;

		if(currentIteration == 0 && patchX == 0.0f && patchY == 0.0f)
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			ShaderManager::useShader("raytracing/geometry_pass");
			shader = ShaderManager::getCurrentShader();

			/* Draw the meshes */
			int entities_count = (int)entities.size();
			renderTarget->bind();
			renderTarget->clear();

			shader->setVariable("viewProjectionMatrix", mCamera->getVPMat());
			mRootEntity->draw(shader);

			GLuint sceneLightsSSBO;
			glGenBuffers(1, &sceneLightsSSBO);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, sceneLightsSSBO);
			glBufferData(
				GL_SHADER_STORAGE_BUFFER,
				sizeof(SceneLight) * mSceneLights.size(),
				&mSceneLights[0],
				GL_DYNAMIC_COPY
			);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, sceneLightsSSBO);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			glFinish();

			lastUpdateTime = Time::gameTime();
			lastIterationTime = Time::gameTime();
		}

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		ShaderManager::useShader("raytracing/raytrace_pass");
		shader = ShaderManager::getCurrentShader();


		shader->setVariable("numLights", (int)mSceneLights.size());


		int fromBuffer = currentIteration % 2;
		int toBuffer = (currentIteration + 1) % 2;

		glm::vec4 currentPatch(patchX, patchY, patchX + patchSizeX, patchY + patchSizeY);

		ShaderManager::useShader("raytracing/raytrace_pass");
		shader = ShaderManager::getCurrentShader();
		shader->setVariable("screenPatch", currentPatch);
		shader->setVariable("positionsTexture", 0);
		shader->setVariable("previousCast", 1);
		shader->setVariable("cameraPosition", mCamera->getPosition());
		shader->setVariable("gameTime", (float)Time::gameTime());
		shader->setVariable("frames", currentIteration);

		renderTarget->getRenderBuffer(0)->bindTexture(0);
		cachedFrame->bind();
		cachedFrame->getRenderBuffer(0)->bindTexture(1);
		overlayQuad->draw();
		renderTarget->getRenderBuffer(0)->unbindTexture();
		cachedFrame->getRenderBuffer(0)->unbindTexture();
		cachedFrame->unbind();
		glFinish();

		/* Blit rendertarget to framebuffer */
		if(Time::gameTime() - lastUpdateTime > 0.2f)
		{
			ShaderManager::useShader("raytracing/blit");
			shader = ShaderManager::getCurrentShader();
			shader->setVariable("newFrame", 0);
			shader->setVariable("screenPatch", currentPatch);
			cachedFrame->getRenderBuffer(0)->bindTexture(0);
			overlayQuad->draw();
			cachedFrame->getRenderBuffer(0)->unbindTexture();
			glFinish();
			SGE::DisplayManager::getDisplayInstance()->swapBuffers();
			lastUpdateTime = Time::gameTime();
		}


		patchX += patchSizeX;
		if(patchX >= 1.0f)
		{
			patchX = 0.0f;
			patchY += patchSizeY;
			if(patchY >= 1.0f)
			{
				patchY = 0.0f;


				float iterationTime = Time::gameTime() - lastIterationTime;
				lastIterationTime = Time::gameTime();
				LOG(DEBUG) << "Draw time (" << iterationTime << "s)";
				currentIteration++;
			}
		}
	}

	std::vector<Scene::SceneLight> Scene::extractLights()
	{
		std::vector<SceneLight> sceneLights;
		recursiveExtractLights(mRootEntity, glm::mat4(1.0f), sceneLights);
		return sceneLights;
	}

	void Scene::recursiveExtractLights(Entity* n, glm::mat4 mat, std::vector<Scene::SceneLight>& sceneLights)
	{
		mat *= n->modelMat;
		std::vector<ILight*> entityLights = n->getLights();
		for(int j = 0; j < entityLights.size(); ++j)
		{
			ILight* l = entityLights[j];
			glm::vec3 lightPosition = l->getPosition();
			glm::vec4 p(lightPosition.x, lightPosition.y, lightPosition.z, 1.0f);
			p = mat * p;

			glm::vec3 c = l->getColor();

			SceneLight sceneLight;
			sceneLight.position = p;
			sceneLight.colour.r = c.r;
			sceneLight.colour.g = c.g;
			sceneLight.colour.b = c.b;
			sceneLight.colour.a = l->getIntensity();
			sceneLights.push_back(sceneLight);
		}

		for(int i = 0; i < n->mChildren.size(); ++i)
		{
			recursiveExtractLights(n->mChildren[i], mat, sceneLights);
		}
	}
}
