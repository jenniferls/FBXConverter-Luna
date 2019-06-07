#include "Loader.h"

Loader::Loader() {

}


Loader::~Loader() {

}

void Loader::PrintData(const char* filePath) {
	reader.readFile(filePath);

	std::cout << std::endl << "Amount of meshes: " << reader.getMeshCount() << std::endl << std::endl;

	for (int i = 0; i < reader.getMeshCount(); i++) {
		mesh = reader.getMesh(i);

		std::cout << "Mesh " << i << std::endl;
		std::cout << "Name: " << mesh.name << std::endl;
		std::cout << "ID: " << mesh.id << std::endl;
		std::cout << "Amount of vertices: " << mesh.vertexCount << std::endl;
		std::cout << "Amount of indices: " << mesh.indexCount << std::endl;
		std::cout << "Has a bounding box: " << (mesh.hasBoundingBox ? "Yes" : "No") << std::endl;
		std::cout << "Has a skeleton: " << (mesh.hasSkeleton ? "Yes" : "No") << std::endl << std::endl;

		reader.getVertices(mesh.id, vertices);
		reader.getIndices(mesh.id, indices);
		Luna::Material mat = reader.getMaterial(mesh.id);

		std::cout << "--- Material ---" << std::endl;
		std::cout << "Diffuse color: " << mat.diffuseVal[0] << ", " << mat.diffuseVal[1] << ", " << mat.diffuseVal[2] << std::endl;
		std::cout << "Ambient color: " << mat.ambientVal[0] << ", " << mat.ambientVal[1] << ", " << mat.ambientVal[2] << std::endl;
		std::cout << "Specular color: " << mat.specularVal[0] << ", " << mat.specularVal[1] << ", " << mat.specularVal[2] << std::endl;
		std::cout << "Diffuse texture: " << mat.diffuseTexPath << std::endl;
		std::cout << "Has normal map: " << (mat.hasNormalMap ? "Yes" : "No") << std::endl;
		if (mat.hasNormalMap) {
			std::cout << "Normal map texture: " << mat.normalTexPath << std::endl;
		}
		std::cout << "Has glow map: " << (mat.hasGlowMap ? "Yes" : "No") << std::endl;
		if (mat.hasGlowMap) {
			std::cout << "Glow map texture: " << mat.glowTexPath << std::endl;
		}

		if (mesh.hasBoundingBox) {
			Luna::BoundingBox bbox = reader.getBoundingBox(mesh.id);
		}

		if (mesh.hasSkeleton) {
			std::vector<Luna::Weights> weights;
			reader.getWeights(mesh.id, weights);
		}
	}

	if (reader.animationExist()) {
		Luna::Skeleton skel = reader.getSkeleton();
		Luna::Animation anim = reader.getAnimation();
		std::vector<Luna::Joint> joints;
		std::vector<Luna::Keyframe> frames;

		reader.getJoints(joints);
		reader.getKeyframes(0, frames);
	}
}
