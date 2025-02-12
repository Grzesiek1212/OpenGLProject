#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include "Mesh.h"
#include <Shader.h>

class Model
{
public:
	explicit Model(const std::string& path)
	{
		loadModel(path);
	}
	void Draw(Shader& shader);
	const std::vector<Mesh>& GetMeshes() const;
private:
	vector<Mesh> meshes;
	string directory;
	vector<Texture>textures_loaded;

	void loadModel(string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	vector<Texture> loadMaterialTextures(aiMaterial* mat,aiTextureType type, string typeName);
};

#endif
