#include "Writer.h"

namespace Luna {
	Writer::Writer() {

	}

	Writer::~Writer() {

	}

	void Writer::setMeshName(unsigned int meshID, const char* name) {
		//const char* newName = name.c_str();
		//this->meshes[meshID].name = newName;
	}

	//void Writer::setMeshPosition(unsigned int meshID, float x, float y, float z) {
	//	this->meshes[meshID].pos[0] = x;
	//	this->meshes[meshID].pos[1] = y;
	//	this->meshes[meshID].pos[2] = z;
	//}

	void Writer::setVertexPosition(Vertex& vertex, float x, float y, float z) {
		vertex.position[0] = x;
		vertex.position[1] = y;
		vertex.position[2] = z;
	}

	void Writer::setVertexNormal(Vertex& vertex, float x, float y, float z) {
		vertex.normal[0] = x;
		vertex.normal[1] = y;
		vertex.normal[2] = z;
	}

	void Writer::setVertexUV(Vertex& vertex, float x, float y) {
		vertex.uv[0] = x;
		vertex.uv[1] = y;
	}

	void Writer::setVertexTangent(Vertex& vertex, float x, float y, float z) {
		vertex.tangent[0] = x;
		vertex.tangent[1] = y;
		vertex.tangent[2] = z;
	}

	void Writer::setVertexBiTangent(Vertex& vertex, float x, float y, float z) {
		vertex.bitangent[0] = x;
		vertex.bitangent[1] = y;
		vertex.bitangent[2] = z;
	}

	void Writer::setMaterialAmbient(Material& mat, float r, float g, float b) {
		mat.ambientVal[0] = r;
		mat.ambientVal[1] = g;
		mat.ambientVal[2] = b;
	}

	void Writer::setMaterialDiffuse(Material& mat, float r, float g, float b) {
		mat.diffuseVal[0] = r;
		mat.diffuseVal[1] = g;
		mat.diffuseVal[2] = b;
	}

	void Writer::setMaterialSpecular(Material& mat, float r, float g, float b) {
		mat.specularVal[0] = r;
		mat.specularVal[1] = g;
		mat.specularVal[2] = b;
	}

	void Writer::write(std::ofstream& outfile, Scene& scene) {
		outfile.write((const char*)&scene, sizeof(Scene));
	}

	void Writer::write(std::ofstream& outfile, Mesh& mesh) {
		outfile.write((const char*)&mesh, sizeof(Mesh) - 2); //To avoid writing junk data we take away the last two bytes added by the compiler
	}

	void Writer::write(std::ofstream& outfile, Vertex& vertex) {
		outfile.write((const char*)&vertex, sizeof(Vertex));
	}

	void Writer::write(std::ofstream& outfile, Index& index) {
		outfile.write((const char*)&index, sizeof(Index));
	}

	void Writer::write(std::ofstream& outfile, Material& mat) {
		outfile.write((const char*)&mat, sizeof(Material) - 2);
	}

	void Writer::write(std::ofstream& outfile, Weights& weights) {
		outfile.write((const char*)&weights, sizeof(Weights));
	}

	void Writer::write(std::ofstream& outfile, Skeleton& skel) {
		outfile.write((const char*)&skel, sizeof(Skeleton));
	}

	void Writer::write(std::ofstream& outfile, Joint& joint) {
		outfile.write((const char*)&joint, sizeof(Joint));
	}

	void Writer::write(std::ofstream& outfile, Animation& anim) {
		outfile.write((const char*)&anim, sizeof(Animation));
	}

	void Writer::write(std::ofstream& outfile, Keyframe& frame) {
		outfile.write((const char*)&frame, sizeof(Keyframe));
	}

	void Writer::write(std::ofstream& outfile, BoundingBox& boundingBox) {
		outfile.write((const char*)&boundingBox, sizeof(BoundingBox));
	}
}