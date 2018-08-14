#ifndef SGE_SCENE_HPP
#define SGE_SCENE_HPP

#include <vector>

#include <sge/time/time.hpp>
#include <sge/input/input.hpp>
#include <sge/model/Entity.hpp>
#include <sge/graphics/IShader.hpp>
#include <sge/graphics/ShaderManager.hpp>
#include <sge/scene/camera.hpp>
#include <sge/scene/geometry/overlayquad.hpp>
#include <sge/scene/bvh/bvh.hpp>
#include <sge/scene/bvh/ssbo.hpp>

namespace SGE
{
	class Export Scene
	{
	private:
		std::vector<Entity*> entities;
		Entity* mRootEntity;
		ShaderManager* shaderManager;

		struct SceneLight{
			glm::vec4 position; // Note: Keep as vec4, because OpenGL will align the buffer to vec4
			glm::vec4 colour;
		};

	public:
		Camera* mCamera;
		OverlayQuad* overlayQuad;
		GLSLRenderTarget* renderTarget;
		GLSLRenderTarget* cachedFrame;
		int currentIteration;
		float lastUpdateTime;
		float lastIterationTime;
		int iterationsPerPatch;
		float patchSizeX;
		float patchSizeY;
		float patchX;
		float patchY;
		Entity* lightDebugModel;
		RT::BVH mBVH;
		RT::BVHSSBO mBVHSSBO;
		std::vector<SceneLight> mSceneLights;

		Scene();

		void addEntity(Entity* entity);
		void update();
		void draw();
		void lightScene();
		std::vector<SceneLight> extractLights();
		void recursiveExtractLights(Entity* n, glm::mat4 mat, std::vector<SceneLight>& l);
	};
}

#endif
