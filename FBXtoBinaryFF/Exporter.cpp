#include "Exporter.h"

Exporter::Exporter() {

}

Exporter::~Exporter() {
	for (int i = 0; i < this->meshVertices.size(); i++) {
		delete (this->meshVertices[i]);
	}
	this->meshVertices.clear();

	for (int i = 0; i < this->meshIndices.size(); i++) {
		delete (this->meshIndices[i]);
	}
	this->meshVertices.clear();

	for (int i = 0; i < this->joints.size(); i++) {
		delete (this->joints[i]);
	}
	this->joints.clear();
}

Luna::Writer Exporter::getWriter() const {
	return this->writer;
}

void Exporter::WriteToBinary(const char* filename){
	std::ofstream outfile(filename, std::ofstream::binary);
	std::cout << std::endl << "Writing to file..." << std::endl;

	writer.write(outfile, this->writer.scene);
	std::cout << "Amount of meshes: " << this->writer.scene.meshCount << std::endl;
	std::cout << "Amount of skeletons: " << this->writer.scene.skeletonCount << std::endl;
	std::cout << "Amount of materials: " << this->writer.scene.materialCount << std::endl;

	for (unsigned int i = 0; i < this->writer.scene.meshCount; i++) {
		writer.write(outfile, this->writer.meshes[i]);
		std::cout << std::endl << "Mesh " << i << std::endl;
		std::cout << this->writer.meshes[i].name << std::endl;
		std::cout << "Vertices: " << this->writer.meshes[i].vertexCount << std::endl;
		std::cout << "Indices: " << this->writer.meshes[i].indexCount << std::endl;
		std::cout << "Skeleton: " << (this->writer.meshes[i].hasSkeleton ? "Yes" : "No") << std::endl;
		std::cout << "Bounding Box: " << (this->writer.meshes[i].hasBoundingBox ? "Yes" : "No") << std::endl;
		std::cout << std::endl;

		for (unsigned int j = 0; j < this->writer.meshes[i].vertexCount; j++) {
			writer.write(outfile, this->meshVertices[i][j]);
		}
		for (unsigned int j = 0; j < this->writer.meshes[i].indexCount; j++) {
			writer.write(outfile, this->meshIndices[i][j]);
		}

		if (this->writer.meshes[i].hasBoundingBox) {
			writer.write(outfile, this->writer.boundingBoxes[i]);
		}
	}

	for (unsigned int i = 0; i < this->writer.scene.materialCount; i++) {
		writer.write(outfile, this->writer.materials[i]);
		std::cout << std::endl << "Material " << i << std::endl;
		std::cout << "Diffuse texture: " << this->writer.materials[i].diffuseTexPath << std::endl;
		std::cout << "Normal map: " << (this->writer.materials[i].hasNormalMap ? "Yes" : "No") << std::endl;
		if (this->writer.materials[i].hasNormalMap) {
			std::cout << "Normal texture: " << this->writer.materials[i].normalTexPath << std::endl;
		}
		std::cout << "Glow map: " << (this->writer.materials[i].hasGlowMap ? "Yes" : "No") << std::endl;
		if (this->writer.materials[i].hasGlowMap) {
			std::cout << "Glow texture: " << this->writer.materials[i].glowTexPath << std::endl;
		}
	}

	for (unsigned int i = 0; i < this->writer.scene.skeletonCount; i++) {
		writer.write(outfile, this->writer.skeletons[i]);
		std::cout << std::endl << "Skeleton " << i << std::endl;
		std::cout << "ID: " << this->writer.skeletons[i].skeletonID << std::endl;
		std::cout << "Amount of joints: " << this->writer.skeletons[i].jointCount << std::endl;
		std::cout << "Amount of animations: " << this->writer.skeletons[i].animationCount << std::endl;

		for (int j = 0; j < this->writer.skeletons[i].jointCount; j++) {
			writer.write(outfile, this->joints[i][j]);
		}
	}

	outfile.close();
}