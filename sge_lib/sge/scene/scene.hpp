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

namespace SGE
{
	class Scene
	{
	private:
		std::vector<Entity*> entities;
		ShaderManager* shaderManager;

		typedef struct{
			ILight* light;
			glm::mat4 modelMat;
		} SceneLight;
		std::vector<SceneLight> sceneLights;

	public:
		Camera* camera;
		OverlayQuad* overlayQuad;
		GLSLRenderTarget* renderTarget;

		Scene();

		void addEntity(Entity* entity);
		void update();
		void draw();
		void lightScene();
	};
}

#endif
