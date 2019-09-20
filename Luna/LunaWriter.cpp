#include "LunaWriter.h"

namespace Luna {
	LunaWriter::LunaWriter() {

	}

	LunaWriter::~LunaWriter() {

	}

	void LunaWriter::setVertexPosition(Vertex& vertex, float x, float y, float z) {
		vertex.position[0] = x;
		vertex.position[1] = y;
		vertex.position[2] = z;
	}

	void LunaWriter::setVertexNormal(Vertex& vertex, float x, float y, float z) {
		vertex.normal[0] = x;
		vertex.normal[1] = y;
		vertex.normal[2] = z;
	}

	void LunaWriter::setVertexUV(Vertex& vertex, float x, float y) {
		vertex.uv[0] = x;
		vertex.uv[1] = y;
	}

	void LunaWriter::setVertexTangent(Vertex& vertex, float x, float y, float z) {
		vertex.tangent[0] = x;
		vertex.tangent[1] = y;
		vertex.tangent[2] = z;
	}

	void LunaWriter::setVertexBiTangent(Vertex& vertex, float x, float y, float z) {
		vertex.bitangent[0] = x;
		vertex.bitangent[1] = y;
		vertex.bitangent[2] = z;
	}

	void LunaWriter::setMaterialAmbient(Material& mat, float r, float g, float b) {
		mat.ambientVal[0] = r;
		mat.ambientVal[1] = g;
		mat.ambientVal[2] = b;
	}

	void LunaWriter::setMaterialDiffuse(Material& mat, float r, float g, float b) {
		mat.diffuseVal[0] = r;
		mat.diffuseVal[1] = g;
		mat.diffuseVal[2] = b;
	}

	void LunaWriter::setMaterialSpecular(Material& mat, float r, float g, float b) {
		mat.specularVal[0] = r;
		mat.specularVal[1] = g;
		mat.specularVal[2] = b;
	}

	void LunaWriter::write(std::ofstream& outfile, Scene& scene) {
		outfile.write((const char*)&scene, sizeof(Scene)/* - 3*/); //To avoid writing junk data
	}

	void LunaWriter::write(std::ofstream& outfile, Mesh& mesh) {
		outfile.write((const char*)&mesh, sizeof(Mesh) - 2); //To avoid writing junk data we take away the last two bytes added by the compiler
	}

	void LunaWriter::write(std::ofstream& outfile, Vertex& vertex) {
		outfile.write((const char*)&vertex, sizeof(Vertex));
	}

	void LunaWriter::write(std::ofstream& outfile, Index& index) {
		outfile.write((const char*)&index, sizeof(Index));
	}

	void LunaWriter::write(std::ofstream& outfile, Material& mat) {
		outfile.write((const char*)&mat, sizeof(Material) - 2);
	}

	void LunaWriter::write(std::ofstream& outfile, Weights& weights) {
		outfile.write((const char*)&weights, sizeof(Weights));
	}

	void LunaWriter::write(std::ofstream& outfile, Skeleton& skel) {
		outfile.write((const char*)&skel, sizeof(Skeleton));
	}

	void LunaWriter::write(std::ofstream& outfile, Joint& joint) {
		outfile.write((const char*)&joint, sizeof(Joint));
	}

	void LunaWriter::write(std::ofstream& outfile, Animation& anim) {
		outfile.write((const char*)&anim, sizeof(Animation));
	}

	void LunaWriter::write(std::ofstream& outfile, Keyframe& frame) {
		outfile.write((const char*)&frame, sizeof(Keyframe));
	}

	void LunaWriter::write(std::ofstream& outfile, BoundingBox& boundingBox) {
		outfile.write((const char*)&boundingBox, sizeof(BoundingBox));
	}

	//void LunaWriter::write(std::ofstream& outfile, Camera& camera) {
	//	outfile.write((const char*)&camera, sizeof(Camera));
	//}
}