#ifndef SGE_ENTITY_HPP
#define SGE_ENTITY_HPP

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Mesh.hpp"
#include "ModelImporter.hpp"

#include <sge/graphics/ShaderManager.hpp>
#include <sge/scripting/objectscript.hpp>

namespace SGE
{
	class Entity
	{
	public:
		std::vector<Mesh*> meshes;
		std::vector<Entity*> mChildren;
		std::vector<ILight*> mLights;
		std::vector<Material*> mMaterials;
		std::vector<ObjectScript*> mAttachedScripts;
		Camera* mCamera = nullptr;
		glm::mat4 modelMat;
		glm::vec3 position;

		void updateTranslationMatrix();


		Export Entity();

		Export bool loadFromFile(std::string file);
		Export bool loadFromFile(std::string file, float scale, bool makeLeftHanded);

		Export void addChild(Entity* n);

		Export void update();
		Export void draw();
		Export void draw(IShader* shader, glm::mat4 parentMat = glm::mat4(1.0f));

		Export void setPositionX(float);
		Export void setPositionY(float);
		Export void setPositionZ(float);
		Export void setPosition(float, float, float);
		Export void setPosition(glm::vec3);

		Export glm::vec3 getPosition();

		Export glm::mat4 getModelMat();

		Export void addLight(ILight* l);
		Export std::vector<ILight*> getLights();
		Export ILight* getLight(int lightIndex);

		Export void attachScript(ObjectScript* script);
	};
}

#endif
