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

	for (int i = 0; i < this->weights.size(); i++) {
		delete (this->weights[i]);
	}
	this->weights.clear();

	for (int i = 0; i < this->keyframes.size(); i++) {
		delete (this->keyframes[i]);
	}
	this->keyframes.clear();
}

Luna::Writer Exporter::getWriter() const {
	return this->writer;
}

void Exporter::WriteToBinary(const char* filename){
	std::cout << std::endl << "Writing to file..." << std::endl;

	std::ofstream outfile(filename, std::ofstream::binary);
	bool hasSkeleton = false;

	writer.write(outfile, this->writer.scene);
	//std::cout << "Amount of meshes: " << this->writer.scene.meshCount << std::endl; //Debug
	//std::cout << "Amount of materials: " << this->writer.scene.materialCount << std::endl; //Debug

	for (unsigned int i = 0; i < this->writer.scene.meshCount; i++) {
		writer.write(outfile, this->writer.meshes[i]);
		//std::cout << std::endl << "Mesh " << i << std::endl; //Debug
		//std::cout << this->writer.meshes[i].name << std::endl; //Debug
		//std::cout << "Vertices: " << this->writer.meshes[i].vertexCount << std::endl; //Debug
		//std::cout << "Indices: " << this->writer.meshes[i].indexCount << std::endl; //Debug
		//std::cout << "Skeleton: " << (this->writer.meshes[i].hasSkeleton ? "Yes" : "No") << std::endl; //Debug
		//std::cout << "Bounding Box: " << (this->writer.meshes[i].hasBoundingBox ? "Yes" : "No") << std::endl; //Debug
		//std::cout << std::endl; //Debug

		for (unsigned int j = 0; j < this->writer.meshes[i].vertexCount; j++) {
			writer.write(outfile, this->meshVertices[i][j]);
		}
		for (unsigned int j = 0; j < this->writer.meshes[i].indexCount; j++) {
			writer.write(outfile, this->meshIndices[i][j]);
		}

		if (this->writer.meshes[i].hasBoundingBox) {
			writer.write(outfile, this->writer.boundingBoxes[i]);
		}

		if (this->writer.meshes[i].hasSkeleton) {
			for (unsigned int j = 0; j < this->writer.meshes[i].vertexCount; j++) {
				hasSkeleton = true;
				writer.write(outfile, this->weights[i][j]);
			}
		}
	}

	for (unsigned int i = 0; i < this->writer.scene.materialCount; i++) {
		writer.write(outfile, this->writer.materials[i]);
		//std::cout << std::endl << "Material " << i << std::endl; //Debug
		//std::cout << "Diffuse texture: " << this->writer.materials[i].diffuseTexPath << std::endl; //Debug
		//std::cout << "Normal map: " << (this->writer.materials[i].hasNormalMap ? "Yes" : "No") << std::endl; //Debug
		if (this->writer.materials[i].hasNormalMap) {
			//std::cout << "Normal texture: " << this->writer.materials[i].normalTexPath << std::endl; //Debug
		}
		//std::cout << "Glow map: " << (this->writer.materials[i].hasGlowMap ? "Yes" : "No") << std::endl; //Debug
		if (this->writer.materials[i].hasGlowMap) {
			//std::cout << "Glow texture: " << this->writer.materials[i].glowTexPath << std::endl; //Debug
		}
	}

	if (hasSkeleton) {
		writer.write(outfile, this->writer.animation);
		//std::cout << std::endl << "Animation" << std::endl; //Debug
		//std::cout << "Name: " << this->writer.animation.animationName << std::endl; //Debug

		writer.write(outfile, this->writer.skeleton);
		//std::cout << std::endl << "Skeleton " << std::endl; //Debug
		//std::cout << "Amount of joints: " << this->writer.skeleton.jointCount << std::endl; //Debug

		for (unsigned int i = 0; i < this->writer.skeleton.jointCount; i++) {
			writer.write(outfile, this->joints[i]);
			//std::cout << "Joint " << i << std::endl; //Debug
			//std::cout << "Joint name: " << this->joints[i].jointName << std::endl; //Debug

			for (unsigned int j= 0; j < this->writer.animation.keyframeCount; j++) {
				writer.write(outfile, this->keyframes[i][j]);
			}
		}
	}

	outfile.close();
}