#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shaderClass.h"
#include "mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glad/glad.h>

#include <string>
#include <vector>
#include "stb_image.h"

class Model
{
public:
	Model(std::string const &path)
	{
		loadModel(path);
	}
	void Draw(Shader& shader);
	

	std::vector<Mesh> meshes;
	std::vector<Texture> textures_loaded;
	std::string directory;

private:


	void loadModel(std::string const &path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
		std::string typeName);

};



//helper function for loading textures from file 

unsigned int TextureFromFile(const char* cstr, std::string const &dir);