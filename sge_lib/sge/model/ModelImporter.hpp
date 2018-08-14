#ifndef SGE_MODEL_IMPORTER_HPP
#define SGE_MODEL_IMPORTER_HPP

#include <iostream>
#include <string>
#include <vector>

#include <easylogging++.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <sge/utils/utils.hpp>
#include <sge/model/ILight.hpp>
#include <sge/model/PointLight.hpp>
#include <sge/model/Mesh.hpp>

#include <sge/scene/camera.hpp>

#include <sge/graphics/texture/ITexture.hpp>
#include <sge/graphics/texture/texturefactory.hpp>



namespace SGE
{
	class ModelImporter
	{
	const aiScene* model;
	std::string mFileName;
	std::string mDirectory;
	std::vector<Mesh*> meshes;
	std::vector<ILight*> lights;
	std::vector<Material*> mMaterials;
	Camera* mCamera = nullptr;

	private:
		void printModelInfo();
		void extractMaterials();
		void extractMaterialTextures(aiMaterial* mat, aiTextureType type, std::vector<ITexture*>& textures);
		void extractTriangles(float scale);
		void _nodeRecurse(aiNode* n, const aiMatrix4x4& m, float scale = 1.0f);
		void _processMesh(aiMesh*, const aiMatrix4x4& m, float scale = 1.0f);
		void extractLights();
		void extractCameras();
		void extractCamera(aiCamera* c);

	public:
		Export ModelImporter();

		Export bool importModel(std::string file);
		Export bool importModel(std::string file, float scale, bool makeLeftHanded);

		Export std::vector<Mesh*> getMeshes();
		Export std::vector<ILight*> getLights();
		Export std::vector<Material*> getMaterials(){ return mMaterials; };

		Export bool hasCamera(){ return mCamera != nullptr; };
		Export Camera* getCamera(){ return mCamera; };
	};
};

#endif
