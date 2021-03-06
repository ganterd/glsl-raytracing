#include "sceneimporter.hpp"

namespace SGE
{
	const char* SceneImporter::_readFile(const char* filePath){
		char* text;

		FILE *file = fopen(filePath, "r");

		if (file == NULL)
			return NULL;

		fseek(file, 0, SEEK_END);
		size_t count = ftell(file);
		rewind(file);


		if (count > 0) {
			text = (char*)malloc(sizeof(char) * (count + 1));
			count = fread(text, sizeof(char), count, file);
			text[count] = '\0';
		}
		fclose(file);

		return text;
	}

	Scene* SceneImporter::importSceneFromFile(std::string sceneFile)
	{
		/* Import the scene text data */
		const char* text = this->_readFile(sceneFile.c_str());
		if(text == NULL)
		{
			LOG(WARNING) << "Couldn't read scene file: " << sceneFile;
			return NULL;
		}

		/* Parse the scene data */
		LOG(INFO) << "Parsing scene file: " << sceneFile;
		tinyxml2::XMLDocument doc;
		doc.Parse((char*)text);

		/* Get the root 'scene' node */
		const tinyxml2::XMLElement* root = doc.FirstChildElement("scene");
		if(root == NULL)
		{
			LOG(WARNING) << "Scene file doesn't have root 'scene' node!";
			return NULL;
		}
		LOG(DEBUG) << "Scene has root node...";
		Scene* scene = new Scene();

		/* Get the scene name, if any */
		std::string sceneName = _getSceneName(root);
		if(sceneName != "")
			LOG(DEBUG) << "Scene name is '" << sceneName << "'";

		std::vector<Entity*> entities = _getSceneEntities(root);
		for(int i = 0; i < entities.size(); ++i)
		{
			scene->addEntity(entities[i]);
		}

		LOG(DEBUG) << "Finished parsing scene";
		return scene;
	}


	std::string SceneImporter::_getSceneName(const tinyxml2::XMLElement* root)
	{
		const tinyxml2::XMLAttribute* nameAttr = root->FindAttribute("name");

		if(nameAttr == NULL)
		{
			LOG(WARNING) << "Scene has no name attribute";
			return "";
		}

		return std::string(nameAttr->Value());
	}

	std::vector<Entity*> SceneImporter::_getSceneEntities(const tinyxml2::XMLElement* root)
	{
		LOG(DEBUG) << "Getting scene entities...";
		std::vector<Entity*> entities;
		const tinyxml2::XMLElement* entitiesNode = root->FirstChildElement("entities");
		if(entitiesNode == NULL)
		{
			LOG(WARNING) << "Scene has no entities node!";
			return entities;
		}

		const tinyxml2::XMLElement* entityNode = entitiesNode->FirstChildElement("entity");
		if(entityNode == NULL)
		{
			LOG(WARNING) << "Scene has no entities!";
			return entities;
		}

		for(; entityNode; entityNode = entityNode->NextSiblingElement("entity"))
		{
			Entity* entity = _getEntity(entityNode);
			if(entity != NULL)
			{
				entities.push_back(entity);
			}
		}

		return entities;
		LOG(DEBUG) << "Finished getting entities...";
	}

	Entity* SceneImporter::_getEntity(const tinyxml2::XMLElement* entityNode)
	{
		Entity* entity;
		LOG(DEBUG) << "Parsing entity...";

		const tinyxml2::XMLElement* modelNode = entityNode->FirstChildElement("model");
		if(modelNode == NULL)
		{
			LOG(WARNING) << "An entity has no model!";
			return NULL;
		}

		const tinyxml2::XMLElement* modelPathNode = modelNode->FirstChildElement("path");
		if(modelPathNode == NULL)
		{
			LOG(ERROR) << "An entity has a model with no path!";
			return NULL;
		}

		std::string modelPath(modelPathNode->GetText());
		LOG(DEBUG) << "Model: " << modelPath;

		std::string scaleString = modelNode->Attribute("scale");
		float scale = 1.0f;
		if(scaleString.length() > 0)
			scale = std::stof(scaleString);
		LOG(DEBUG) << "Scale: " << scale;

		entity = new Entity();
		entity->loadFromFile(modelPath, scale, false);

		LOG(DEBUG) << "Finished parsing entity";
		return entity;
	}
}
