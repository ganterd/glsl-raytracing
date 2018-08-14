#include "Entity.hpp"

namespace SGE
{
	Entity::Entity()
	{
		this->modelMat = glm::mat4(1.0f);
	}

	bool Entity::loadFromFile(std::string file)
	{
		return this->loadFromFile(file, 1.0f, false);
	}

	bool Entity::loadFromFile(std::string file, float scale,  bool makeLeftHanded)
	{
		ModelImporter* importer = new ModelImporter();
		importer->importModel(file, scale, makeLeftHanded);
		this->meshes = importer->getMeshes();
		mLights = importer->getLights();
		mMaterials = importer->getMaterials();
		if(importer->hasCamera())
			mCamera = importer->getCamera();

		return true;
	}

	void Entity::addChild(Entity* n)
	{
		mChildren.push_back(n);
	}

	void Entity::attachScript(ObjectScript* script)
	{
		mAttachedScripts.push_back(script);
		script->mEntity = this;
	}

	void Entity::update()
	{
		for(int i = 0; i < mAttachedScripts.size(); ++i)
		{
			mAttachedScripts[i]->update();
		}
		for(int c = 0; c < mChildren.size(); ++c)
			mChildren[c]->update();
	}

	void Entity::draw()
	{
		for(int i = 0; i < this->meshes.size(); ++i)
		{
			this->meshes[i]->renderGL();
		}
	}

	void Entity::draw(IShader* shader, glm::mat4 currentMat)
	{
		currentMat *= modelMat;
		shader->setVariable("modelMatrix", currentMat);
		for(int i = 0; i < this->meshes.size(); ++i)
			this->meshes[i]->renderGL();

		for(int i = 0; i < mChildren.size(); ++i)
			mChildren[i]->draw(shader, currentMat);
	}

	void Entity::setPositionX(float x)
	{
		this->setPosition(glm::vec3(x, this->position.y, this->position.z));
	}

	void Entity::setPositionY(float y)
	{
		this->setPosition(glm::vec3(this->position.x, y, this->position.z));
	}

	void Entity::setPositionZ(float z)
	{
		this->setPosition(glm::vec3(this->position.x, this->position.y, z));
	}

	void Entity::setPosition(float x, float y, float z)
	{
		this->setPosition(glm::vec3(x, y, z));
	}

	void Entity::setPosition(glm::vec3 p)
	{
		this->position = p;
		this->updateTranslationMatrix();
	}

	glm::vec3 Entity::getPosition()
	{
		return this->position;
	}

	void Entity::updateTranslationMatrix()
	{
		// TODO: Rotation & Scale
		glm::mat4 mat = glm::translate(glm::mat4(1.0f), this->position);
		this->modelMat = mat;
	}

	glm::mat4 Entity::getModelMat()
	{
		return this->modelMat;
	}

	void Entity::addLight(ILight* l)
	{
		l->mParent = this;
		mLights.push_back(l);
	}

	std::vector<ILight*> Entity::getLights()
	{
		return mLights;
	}

	ILight* Entity::getLight(int lightIndex)
	{
		// Not my fault if you index wrong...
		return mLights[lightIndex];
	}
}
