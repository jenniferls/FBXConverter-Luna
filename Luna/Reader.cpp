#include "Reader.h"

namespace Luna {
	Reader::Reader() {
		//this->meshVertices = { nullptr };
		//this->meshIndices = { nullptr };
	}

	Reader::~Reader() {
		clean();
	}

	void Reader::readFile(const char* filePath){
		std::ifstream infile(filePath, std::ifstream::binary);
		//TODO: Clean the reader

		read(infile, this->scene); //Read the scene header
		this->meshCount = this->scene.meshCount;
		this->materialCount = this->scene.materialCount;
		this->skeletonCount = this->scene.skeletonCount;
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
			if (this->meshes[i].hasBoundingBox) {
				this->boundingBoxes.push_back(BoundingBox());
				read(infile, this->boundingBoxes[i]);
			}
		}
		this->materials.resize(this->materialCount);
		for (unsigned int i = 0; i < this->materialCount; i++) {
			read(infile, this->materials[i]);
		}
		this->skeletons.resize(this->skeletonCount);
		for (unsigned int i = 0; i < this->scene.skeletonCount; i++) {
			read(infile, this->skeletons[i]);
		}

		infile.close();
	}

	void Reader::read(std::ifstream& infile, Scene& scene){
		infile.read((char*)&scene, sizeof(Scene));
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

	void Reader::clean() {
		this->meshCount = 0;
		this->materialCount = 0;
		this->skeletonCount = 0;
		this->meshes.clear();
		this->skeletons.clear();

		for (int i = 0; i < this->meshVertices.size(); i++) {
			delete (this->meshVertices[i]);
		}
		this->meshVertices.clear();

		for (int i = 0; i < this->meshIndices.size(); i++) {
			delete (this->meshIndices[i]);
		}
		this->meshIndices.clear();

		for (int i = 0; i < this->joints.size(); i++) {
			delete (this->joints[i]);
		}
		this->joints.clear();
	}

	unsigned int Reader::getMeshCount() const {
		return meshCount;
	}

	unsigned int Reader::getMaterialCount() const {
		return this->materialCount;
	}

	unsigned int Reader::getSkeletonCount() const {
		return this->skeletonCount;
	}

	Mesh Reader::getMesh(int id) const {
		return this->meshes[id];
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

	Skeleton Reader::getSkeleton(int id) const {
		return this->skeletons[id];
	}
}