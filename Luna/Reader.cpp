#include "Reader.h"

namespace Luna {
	Reader::Reader() {
		this->hasAnimation = false;
	}

	Reader::~Reader() {
		clean();
	}

	bool Reader::readFile(const char* filePath){
		bool success = false;
		try{

			std::ifstream infile(filePath, std::ifstream::binary);

			std::string str(filePath);

			std::string extention = ".lu";

			std::string end = str.substr(str.size() - 3, str.size());

			if (infile && (end == extention)){
				//TODO: Clean the reader

				read(infile, this->scene); //Read the scene header
				this->meshCount = this->scene.meshCount;
				this->materialCount = this->scene.materialCount;
				if (this->scene.hasCamera == true) {
					read(infile, this->camera);
				}
				this->meshes.resize(this->scene.meshCount);
				for (unsigned int i = 0; i < this->scene.meshCount; i++) { //For every mesh in the scene, read in mesh header
					read(infile, this->meshes[i]);
					this->meshVertices.push_back(new Vertex[this->meshes[i].vertexCount]);
					this->meshIndices.push_back(new Index[this->meshes[i].indexCount]);
					for (unsigned int j = 0; j < this->meshes[i].vertexCount; j++) { //For every vertex in the mesh, read in vertex header
						read(infile, this->meshVertices[i][j]); //One Mesh
					}
					for (unsigned int j = 0; j < this->meshes[i].indexCount; j++) { //For every index in the mesh, read in index header
						read(infile, this->meshIndices[i][j]); //One Index
					}
					if (this->meshes[i].hasBoundingBox) { //If the mesh has a bounding box, read in its' bounding box
						this->boundingBoxes.push_back(BoundingBox());
						read(infile, this->boundingBoxes[i]); //One bounding box
					}
					if (this->meshes[i].hasSkeleton) {
						this->hasAnimation = true;
						this->weights.push_back(new Weights[this->meshes[i].vertexCount]);
						for (unsigned int j = 0; j < this->meshes[i].vertexCount; j++) {
							read(infile, this->weights[i][j]);
						}
					}
				}
				this->materials.resize(this->materialCount);
				for (unsigned int i = 0; i < this->materialCount; i++) {
					read(infile, this->materials[i]);
				}
				if (hasAnimation) {
					read(infile, this->animation);
					read(infile, this->skeleton);
					this->joints.resize(this->skeleton.jointCount);
					for (unsigned int i = 0; i < this->skeleton.jointCount; i++) {
						read(infile, this->joints[i]);
						this->keyframes.push_back(new Keyframe[this->animation.keyframeCount]);
						for (unsigned int j = 0; j < this->animation.keyframeCount; j++) {
							read(infile, this->keyframes[i][j]);
						}
					}
				}
				success = true;
			}
			else{
				OutputDebugStringW(L"The Filepath fed to the reader could not be opened. Either wrong filetype or faulty path. Please ensure file is of .lu type.");
			}
			infile.close();
		}
		catch (int n){
			throw;
		}
		return success;
	}

	void Reader::read(std::ifstream& infile, Scene& scene){
		infile.read((char*)&scene, sizeof(Scene) - 3);
	}

	void Reader::read(std::ifstream& infile, Mesh& mesh){
		infile.read((char*)&mesh, sizeof(Mesh) - 2);
	}

	void Reader::read(std::ifstream& infile, Vertex& vertex) {
		infile.read((char*)&vertex, sizeof(Vertex));
	}

	void Reader::read(std::ifstream& infile, Index& index){
		infile.read((char*)&index, sizeof(Index));
	}

	void Reader::read(std::ifstream& infile, Material& mat){
		infile.read((char*)&mat, sizeof(Material) - 2);
	}

	void Reader::read(std::ifstream& infile, Weights& weights) {
		infile.read((char*)&weights, sizeof(Weights));
	}

	void Reader::read(std::ifstream& infile, Skeleton& skeleton) {
		infile.read((char*)&skeleton, sizeof(Skeleton));
	}

	void Reader::read(std::ifstream& infile, Joint& joint) {
		infile.read((char*)&joint, sizeof(Joint));
	}

	void Reader::read(std::ifstream& infile, Animation& anim) {
		infile.read((char*)&anim, sizeof(Animation));
	}

	void Reader::read(std::ifstream& infile, Keyframe& frame) {
		infile.read((char*)&frame, sizeof(Keyframe));
	}

	void Reader::read(std::ifstream& infile, BoundingBox& boundingBox) {
		infile.read((char*)&boundingBox, sizeof(BoundingBox));
	}

	void Reader::read(std::ifstream& infile, Camera& camera) {
		infile.read((char*)&camera, sizeof(Camera));
	}

	void Reader::clean() {
		this->meshCount = 0;
		this->materialCount = 0;
		this->hasAnimation = false;
		this->meshes.clear();
		this->joints.clear();

		for (int i = 0; i < this->meshVertices.size(); i++) {
			delete (this->meshVertices[i]);
		}
		this->meshVertices.clear();

		for (int i = 0; i < this->meshIndices.size(); i++) {
			delete (this->meshIndices[i]);
		}
		this->meshIndices.clear();

		for (int i = 0; i < this->weights.size(); i++) {
			delete (this->weights[i]);
		}
		this->weights.clear();

		for (int i = 0; i < this->keyframes.size(); i++) {
			delete (this->keyframes[i]);
		}
		this->keyframes.clear();
	}

	unsigned int Reader::getMeshCount() const {
		return meshCount;
	}

	unsigned int Reader::getMaterialCount() const {
		return this->materialCount;
	}

	bool Reader::getHasCamera() const {
		return this->scene.hasCamera;
	}

	Mesh Reader::getMesh(int id) const {
		return this->meshes[id];
	}

	Skeleton Reader::getSkeleton() const {
		return this->skeleton;
	}

	void Reader::getVertices(int meshID, std::vector<Vertex>& vertices) {
		vertices.resize(this->meshes[meshID].vertexCount);
		for (unsigned int i = 0; i < this->meshes[meshID].vertexCount; i++) {
			vertices[i] = this->meshVertices[meshID][i];
		}
	}

	void Reader::getIndices(int meshID, std::vector<Index>& indices) {
		indices.resize(this->meshes[meshID].indexCount);
		for (unsigned int i = 0; i < this->meshes[meshID].indexCount; i++) {
			indices[i] = this->meshIndices[meshID][i];
		}
	}

	Material Reader::getMaterial(int meshID) const {
		return this->materials[meshID];
	}

	void Reader::getMaterials(std::vector<Material>& materials) {
		materials.resize(this->materialCount);
		for (unsigned int i = 0; i < this->materialCount; i++) {
			materials[i] = this->materials[i];
		}
	}

	BoundingBox Reader::getBoundingBox(int meshID) const {
		return this->boundingBoxes[meshID];
	}

	void Reader::getWeights(int meshID, std::vector<Weights>& weights) {
		weights.resize(this->meshes[meshID].vertexCount);
		for (unsigned int i = 0; i < this->meshes[meshID].vertexCount; i++) {
			weights[i] = this->weights[meshID][i];
		}
	}

	void Reader::getJoints(std::vector<Joint>& joints) {
		joints.resize(this->skeleton.jointCount);
		for (unsigned int i = 0; i < this->skeleton.jointCount; i++) {
			joints[i] = this->joints[i];
		}
	}

	void Reader::getKeyframes(int jointID, std::vector<Keyframe>& keyframes) {
		keyframes.resize(this->animation.keyframeCount);
		for (unsigned int i = 0; i < this->animation.keyframeCount; i++) {
			keyframes[i] = this->keyframes[jointID][i];
		}
	}

	bool Reader::animationExist() const {
		return this->hasAnimation;
	}

	Animation Reader::getAnimation() const {
		return this->animation;
	}

	Camera Reader::getCamera() const {
		return this->camera;
	}
}