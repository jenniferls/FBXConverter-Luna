#pragma once
#include "../Luna/Luna.h"
#include "FBXExporter.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <FBXSDK/include/fbxsdk.h>

class FBXReader {
public:
	FBXReader();
	~FBXReader();

	void LoadFBX(const char* filename);
	void ConvertFBX(FBXExporter* exporter, const char* outPath);
	std::string InputAnimName();

private:
	typedef struct SkinData { //Temporary datatype for gathering weights
		unsigned int jointID[4]{ 0 };
		float		 weight[4]{ 0.0f };
		int			 minWeightIndex = 0;
		float		 minWeight = 0.0f;
	} SkinData;

	FbxManager* fbxManager;
	FbxScene* scene;
	FbxIOSettings* ioSettings;
	FbxNode* rootNode;

	std::string outputFile;
	std::experimental::filesystem::path outputPath;

	//std::vector<Luna::Vertex*> meshVertices;
	//std::vector<Luna::Index*> meshIndices;

	void GetData(FbxNode* node, FBXExporter* exporter);
	void GetMeshData(FbxMesh* mesh, FBXExporter* exporter);
	void GetMaterialData(FbxMesh* mesh, FBXExporter* exporter);
	bool GetBoundingBoxData(FbxMesh* mesh, FBXExporter* exporter);
	bool CreateBoundingBox(FbxMesh* mesh, FBXExporter* exporter);
	void GetWeightsData(FbxMesh* fbxmesh, unsigned int meshID, FBXExporter* exporter);
	void GetAnimationData(FbxMesh* fbxmesh, unsigned int meshID, FBXExporter* exporter);

	unsigned int GetJointIdByName(const char* jointName, FBXExporter* exporter, unsigned int meshID);

	bool isBoundingBox(FbxNode* node);
	bool hasSkeleton(FbxNode* node);
	bool isTriangulated(FbxMesh* mesh);
	void triangulating(FbxMesh* mesh, FBXExporter* exporter,std::vector<Luna::Vertex>* vertices);
};