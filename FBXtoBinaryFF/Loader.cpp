#include "Loader.h"

Loader::Loader() {

}


Loader::~Loader() {

}

void Loader::PrintData(const char* filePath) {
	reader.readFile(filePath);

	for (int i = 0; i < reader.getMeshCount(); i++) {
		reader.getVertices(i, vertices);
		reader.getIndices(i, indices);

		mesh = reader.getMesh(i);

		if (mesh.hasBoundingBox) {
			bbox = reader.getBoundingBox(mesh.id);
		}

		if (mesh.hasSkeleton) {
			std::vector<Luna::Weights> weights;
			reader.getWeights(mesh.id, weights);
		}
	}

	reader.getMaterials(materials);

	if (reader.animationExist()) {
		Luna::Skeleton skel = reader.getSkeleton();
		Luna::Animation anim = reader.getAnimation();
		std::vector<Luna::Joint> joints;
		std::vector<Luna::Keyframe> frames;

		reader.getJoints(joints);
		reader.getKeyframes(0, frames);
	}
}
