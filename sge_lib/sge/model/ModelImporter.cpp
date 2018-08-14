#include "ModelImporter.hpp"

namespace SGE
{
	ModelImporter::ModelImporter()
	{
	}

	bool ModelImporter::importModel(std::string file)
	{
		return this->importModel(file, 1.0f, false);
	}

	bool ModelImporter::importModel(std::string file, float scale, bool makeLeftHanded)
	{
		LOG(INFO) << " Loading model: " << file ;
		LOG(DEBUG) << " |- Make Left Handed: " << (makeLeftHanded ? "true" : "false");

		Assimp::Importer importer;
		model = importer.ReadFile(file,
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_FlipUVs |
			aiProcess_CalcTangentSpace |
			(makeLeftHanded ? aiProcess_MakeLeftHanded : 0)
		);

		if(!model)
		{
			LOG(ERROR) << "Error importing '" << file
				<< "': " << importer.GetErrorString() ;
			return false;
		}

		Utils::splitFilename(file, mDirectory, mFileName);

		this->printModelInfo();

		/* Initialise the Matrials for the model */
		this->extractMaterials();

		/* Create the VAOs for the model */
		this->extractTriangles(scale);

		/* Create light sources for the model */
		this->extractLights();

		this->extractCameras();

		return true;
	}

	void ModelImporter::extractCameras()
	{
		if(model->mNumCameras)
			extractCamera(model->mCameras[0]);
	}

	void ModelImporter::extractCamera(aiCamera* c)
	{

		aiVector3D cp = c->mPosition;
		aiVector3D cl = c->mLookAt;
		aiVector3D cu = c->mUp;

		aiNode* cn = model->mRootNode->FindNode(c->mName);

		aiMatrix4x4 ct = cn->mTransformation;

		cn = cn->mParent;
		while(cn->mParent)
		{
			ct = cn->mTransformation * ct;
			cn = cn->mParent;
		}

		aiMatrix3x3 rotationMatrix(ct);

		cp *= ct;
		cl *= ct;
		cu *= rotationMatrix;

		mCamera = new Camera();
		mCamera->setPosition(glm::vec3(cp.x, cp.y, cp.z));
		mCamera->setLookAt(glm::vec3(cl.x, cl.y, cl.z));
		mCamera->setUpVector(glm::vec3(cu.x, cu.y, cu.z));
		mCamera->setFoV(c->mHorizontalFOV);


	}

	void ModelImporter::printModelInfo()
	{
		LOG(INFO) << " |-Textures: " << model->mNumTextures;
		LOG(INFO) << " |-Cameras: " << model->mNumCameras;
		LOG(INFO) << " |-Animations: " << model->mNumAnimations;
	}

	void ModelImporter::extractMaterials()
	{
		LOG(INFO) << " |-Materials: " << model->mNumMaterials;
		for(unsigned int i = 0; i < model->mNumMaterials; ++i)
		{
			/* Get textures */
			SGE::Material* newMaterial = new SGE::Material();
			aiMaterial* material = model->mMaterials[i];

			aiString matName;
			material->Get<aiString>(AI_MATKEY_NAME, matName);
			LOG(DEBUG) << "  |- [" << i << "] '" << matName.C_Str() << "'";

			std::vector<ITexture*> textures;
			for(int t = aiTextureType_DIFFUSE; t <= aiTextureType_UNKNOWN; ++t)
			{
				extractMaterialTextures(material, (aiTextureType)t, textures);
			}

			for(int t = 0; t < textures.size(); ++t)
			{
				newMaterial->addTexture(textures[t]);
			}
			mMaterials.push_back(newMaterial);
		}
	}

	void ModelImporter::extractMaterialTextures(
		aiMaterial* mat,
		aiTextureType type,
		std::vector<ITexture*>& textures
	) {
		aiString p;
		for(unsigned int j = 0; j < mat->GetTextureCount(type); ++j)
		{
			mat->GetTexture(type, j, &p);
			std::string path = mDirectory + "/" + std::string(p.C_Str());
			ITexture* tex = NULL;
			switch(type)
			{
				case aiTextureType_DIFFUSE:
					LOG(INFO) << "   |- Diffuse: " << path;
					tex = TextureFactory::newTexture();
					tex->loadFromFile(path, ITexture::Type::Diffuse);
					break;
				case aiTextureType_SPECULAR:
					LOG(INFO) << "   |- Specular: " << path;
					tex = TextureFactory::newTexture();
					tex->loadFromFile(path, ITexture::Type::Specular);
					break;
				case aiTextureType_AMBIENT:
					LOG(INFO) << "   |- Ambient: " << path;
					break;
				case aiTextureType_EMISSIVE:
					LOG(INFO) << "   |- Emissive: " << path;
					break;
				case aiTextureType_HEIGHT:
					LOG(INFO) << "   |- Height: " << path;
					break;
				case aiTextureType_NORMALS:
					LOG(INFO) << "   |- Normals: " << path;
					tex = TextureFactory::newTexture();
					tex->loadFromFile(path, ITexture::Type::Normals);
					break;
				case aiTextureType_SHININESS:
					LOG(INFO) << "   |- Shininess: " << path;
					break;
				case aiTextureType_OPACITY:
					LOG(INFO) << "   |- Opacity: " << path;
					tex = TextureFactory::newTexture();
					tex->loadFromFile(path, ITexture::Type::Opacity);
					break;
				case aiTextureType_DISPLACEMENT:
					LOG(INFO) << "   |- Displacement: " << path;
					break;
				case aiTextureType_LIGHTMAP:
					LOG(INFO) << "   |- Lightmap (AO): " << path;
					break;
				case aiTextureType_REFLECTION:
					LOG(INFO) << "   |- Reflection: " << path;
					break;
				case aiTextureType_UNKNOWN:
					LOG(WARNING) << "   |- Unknown texture type (" << path << ")";
				default:
					LOG(WARNING) << "   |- Unhandled texture type (" << path << ")";
			}

			if(tex)
			{
				textures.push_back(tex);
			}
		}
	}

	void ModelImporter::extractTriangles(float scale)
	{
		LOG(INFO) << " |-Meshes: " << model->mNumMeshes ;

		aiMatrix4x4 identity;
		_nodeRecurse(model->mRootNode, identity, scale);
	}

	void ModelImporter::_nodeRecurse(aiNode* n, const aiMatrix4x4& t, float scale)
	{
		aiMatrix4x4 currentTransform =  n->mTransformation * t;
		for(unsigned int m = 0; m < n->mNumMeshes; ++m)
		{
			_processMesh(model->mMeshes[n->mMeshes[m]], currentTransform, scale);
		}

		for(unsigned int c = 0; c < n->mNumChildren; ++c)
		{
			_nodeRecurse(n->mChildren[c], currentTransform, scale);
		}
	}

	void ModelImporter::_processMesh(aiMesh* mesh, const aiMatrix4x4& m, float scale)
	{
		GLfloat* meshVertexData = new GLfloat[mesh->mNumVertices * 3];
		GLfloat* meshNormalsData = new GLfloat[mesh->mNumVertices * 3];
		GLfloat* meshTangentsData = new GLfloat[mesh->mNumVertices * 3];
		SGE::Mesh* resultMesh = new SGE::Mesh();

		/* Direct copy VBO from mesh */
		for(unsigned int j = 0; j < mesh->mNumVertices; ++j)
		{
			aiVector3D v = m * (mesh->mVertices[j] * scale);
			//aiVector3D v = mesh->mVertices[j] * scale;
			meshVertexData[(j * 3) + 0] = v.x;
			meshVertexData[(j * 3) + 1] = v.y;
			meshVertexData[(j * 3) + 2] = v.z;
		}
		resultMesh->setVBOData(meshVertexData, mesh->mNumVertices);

		if(mesh->HasNormals())
		{
			for(unsigned int j = 0; j < mesh->mNumVertices; ++j)
			{
				meshNormalsData[(j * 3) + 0] = mesh->mNormals[j].x;
				meshNormalsData[(j * 3) + 1] = mesh->mNormals[j].y;
				meshNormalsData[(j * 3) + 2] = mesh->mNormals[j].z;
			}
			resultMesh->setNBOData(meshNormalsData, mesh->mNumVertices);
		}
		if(mesh->HasTangentsAndBitangents())
		{
			for(unsigned int j = 0; j < mesh->mNumVertices; ++j)
			{
				meshTangentsData[(j * 3) + 0] = mesh->mTangents[j].x;
				meshTangentsData[(j * 3) + 1] = mesh->mTangents[j].y;
				meshTangentsData[(j * 3) + 2] = mesh->mTangents[j].z;
			}
			resultMesh->setTangentsData(meshTangentsData, mesh->mNumVertices);
		}

		/* Extract the VBI from faces */
		unsigned int* meshIndexData = new unsigned int[mesh->mNumFaces * 3];
		for(unsigned int j = 0; j < mesh->mNumFaces; ++j)
		{
			meshIndexData[(j * 3) + 0] = mesh->mFaces[j].mIndices[0];
			meshIndexData[(j * 3) + 1] = mesh->mFaces[j].mIndices[1];
			meshIndexData[(j * 3) + 2] = mesh->mFaces[j].mIndices[2];
		}
		resultMesh->setIBOData(meshIndexData, mesh->mNumFaces);

		resultMesh->setNBOData(meshNormalsData, mesh->mNumVertices);

		if(mesh->HasTextureCoords(0))
		{
			GLfloat* meshUVData = nullptr;
			meshUVData = new GLfloat[mesh->mNumVertices * 2];
			for(unsigned int j = 0; j < mesh->mNumVertices; ++j)
			{
				meshUVData[(j * 2) + 0] = mesh->mTextureCoords[0][j].x;
				meshUVData[(j * 2) + 1] = mesh->mTextureCoords[0][j].y;
			}
			resultMesh->setUVData(meshUVData, mesh->mNumVertices);
		}

		/* Create a local mesh */
		resultMesh->setMaterial(mMaterials[mesh->mMaterialIndex]);


		meshes.push_back(resultMesh);
	}

	void ModelImporter::extractLights()
	{
		LOG(INFO) << " |-Lights: " << model->mNumLights;
		for(unsigned int i = 0; i < model->mNumLights; ++i)
		{
			LOG(DEBUG) << "   |-Light " << i ;
			aiLight* light = model->mLights[i];
			aiNode* node = model->mRootNode->FindNode(light->mName);
			aiMatrix4x4 transform = node->mTransformation;
			node = node->mParent;
			while(node != model->mRootNode)
			{
				transform = node->mTransformation * transform;
				node = node->mParent;
			}
			aiMatrix3x3 rotation(transform);

			// Determine and transform positions and vectors
			aiVector3D pos = transform * light->mPosition;
			aiVector3D dir = transform * light->mDirection - pos;
			aiVector3D up = transform * light->mUp - pos;
			aiVector2D area = light->mSize;
			aiColor3D diff = light->mColorDiffuse;
			aiColor3D spec = light->mColorSpecular;
			aiColor3D amb = light->mColorAmbient;

			glm::vec3 colour = glm::vec3(diff.r, diff.g, diff.b);
			float colourPower = std::max(std::max(colour.r, colour.g), colour.b);
			colour /= colourPower;

			ILight* l = new ILight();
			l->setPosition(glm::vec3(pos.x, pos.y, pos.z));
			l->setDirection(glm::vec3(dir.x, dir.y, dir.z));
			l->setUpVector(glm::vec3(up.x, up.y, up.z));
			l->setSize(glm::vec2(area.x, area.y));
			l->setColor(colour);
			l->setIntensity(colourPower);

			float atten0 = light->mAttenuationConstant;
			float atten1 = light->mAttenuationLinear;
			float atten2 = light->mAttenuationQuadratic;
			LOG(DEBUG) << "     |- Name: '" << light->mName.C_Str() << "'";
			LOG(DEBUG) << "     |- Atten0: " << atten0;
			LOG(DEBUG) << "     |- Atten1: " << atten1;
			LOG(DEBUG) << "     |- Atten2: " << atten2;
			LOG(DEBUG) << "     |- Diffuse: " << colour.r << ", " << colour.g << ", " << colour.b;
			LOG(DEBUG) << "     |- Power: " << colourPower;

			if(light->mType == aiLightSource_POINT)
			{
				LOG(DEBUG) << "     |-Type: Point";
				l->setType(ILight::Point);
			}
			else if(light->mType == aiLightSource_AREA)
			{
				LOG(DEBUG) << "     |-Type: Area";
				l->setType(ILight::Area);
			}

			lights.push_back(l);
		}
	}

	std::vector<Mesh*> ModelImporter::getMeshes()
	{
		return this->meshes;
	}

	std::vector<ILight*> ModelImporter::getLights()
	{
		return this->lights;
	}
}
