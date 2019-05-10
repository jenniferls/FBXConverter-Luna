#pragma once
#include "Luna/Luna.h"
#include "Exporter.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <FBXSDK/include/fbxsdk.h>

class Reader {
public:
	Reader();
	~Reader();

	void LoadFBX(const char* filename);
	void ConvertFBX(Exporter* exporter, const char* outPath);

private:
	FbxManager* fbxManager;
	FbxScene* scene;
	FbxIOSettings* ioSettings;
	FbxNode* rootNode;

	std::string outputFile;
	std::experimental::filesystem::path outputPath;

	//std::vector<Luna::Vertex*> meshVertices;
	//std::vector<Luna::Index*> meshIndices;

	void GetData(FbxNode* node, Exporter* exporter);
	void GetMeshData(FbxMesh* mesh, Exporter* exporter);
	void GetMaterialData(FbxMesh* mesh, Exporter* exporter);
	bool GetBoundingBoxData(FbxMesh* mesh, Exporter* exporter);
	void GetSkeletonData(FbxNode* node, Exporter* exporter);
	void GetWeightsData(FbxMesh* fbxmesh, Luna::Skeleton& skel, unsigned int meshID, Exporter* exporter);

	unsigned int GetJointIdByName(const char* jointName, Exporter* exporter, unsigned int meshID);

	bool isBoundingBox(FbxNode* node);
	bool hasSkeleton(FbxNode* node);
	bool isTriangulated(FbxMesh* mesh);
};