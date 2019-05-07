#include "Reader.h"

Reader::Reader() {
	fbxManager = nullptr;
	scene = nullptr;
	ioSettings = nullptr;
	rootNode = nullptr;
}

Reader::~Reader() {
	if (ioSettings) {
		ioSettings->Destroy();
	}
	if (scene) {
		scene->Destroy();
	}
	if (fbxManager) {
		fbxManager->Destroy();
	}
}

void Reader::LoadFBX(const char* filename){
	fbxManager = FbxManager::Create();
	ioSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
	fbxManager->SetIOSettings(ioSettings);

	//Settings
	fbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_MATERIAL, true);
	fbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_TEXTURE, true);
	fbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_LINK, false);
	fbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_SHAPE, false);
	fbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_GOBO, false);
	fbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_ANIMATION, true);
	fbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);

	FbxImporter* importer = FbxImporter::Create(fbxManager, "");
	bool importStatus = importer->Initialize(filename, -1, fbxManager->GetIOSettings()); //Initialize the importer and make sure no error occured.
	if (!importStatus) {
		std::cout << "Error loading fbx-file! " << std::endl;
		throw (importer->GetStatus().GetErrorString());
		return;
	}
	
	scene = FbxScene::Create(fbxManager, "Imported Scene");
	importer->Import(this->scene);
	importer->Destroy(); //Safe to free up memory now
	rootNode = scene->GetRootNode();
}

void Reader::ConvertFBX(Exporter* exporter, const char* outPath){
	this->outputFile = outPath;
	this->outputPath = this->outputFile;
	this->outputPath = this->outputPath.remove_filename();

	GetData(this->rootNode, exporter);
	GetSkeletonData(this->rootNode, exporter);
}

void Reader::GetData(FbxNode* node, Exporter* exporter) {
	std::cout << std::endl; //Debug
	std::cout << node->GetName() << std::endl; //Debug
	std::cout << node->GetChildCount() << std::endl; //Debug

	for (int i = 0; i < node->GetChildCount(); i++) {
		GetData(node->GetChild(i), exporter); //Recursive
	}

	FbxMesh* mesh = node->GetMesh();

	if (mesh && !isBoundingBox(node)) { //If the node is a mesh that is not a bounding box
		GetMeshData(mesh, exporter);
	}
}

void Reader::GetMeshData(FbxMesh* mesh, Exporter* exporter) {
	std::cout << "This is a mesh!" << std::endl; //Debug
	if (!isTriangulated(mesh)) {
		throw("Error: The mesh is not triangulated!");
	}
	Luna::Mesh tempMesh; //Create temporary mesh and fill with info

	memcpy(tempMesh.name, mesh->GetNode()->GetName(), NAME_SIZE);
	tempMesh.id = exporter->writer.scene.meshCount;
	tempMesh.pos[0] = (float)mesh->GetNode()->LclTranslation.Get()[0];
	tempMesh.pos[1] = (float)mesh->GetNode()->LclTranslation.Get()[1];
	tempMesh.pos[2] = (float)mesh->GetNode()->LclTranslation.Get()[2];
	tempMesh.hasSkeleton = hasSkeleton(mesh->GetNode());

	if (tempMesh.hasSkeleton) {
		std::cout << "The mesh has a skeleton!" << std::endl; //Debug
		GetWeightsData(mesh, exporter);
		exporter->writer.scene.skeletonCount += 1;
	}

	std::vector<int> vertIndices;
	int vertIndexCount = mesh->GetPolygonVertexCount();
	vertIndices.resize(vertIndexCount);
	for (int i = 0; i < vertIndexCount; i++) {
		vertIndices[i] = mesh->GetPolygonVertices()[i];
	}

	std::vector<Luna::Vertex> tempVertices; //Temporary vertex array (only push unique vertices to the vector)

	FbxVector4* controlPoints = mesh->GetControlPoints();
	FbxArray<FbxVector4> fbxNormals;
	mesh->GetPolygonVertexNormals(fbxNormals);

	std::vector<int> fbxUVIndices;
	for (int i = 0; i < mesh->GetPolygonCount(); ++i) //Gather the UV indices
	{
		for (int j = 0; j < 3; ++j)
		{
			int UVIndex = mesh->GetTextureUVIndex(i, j);
			fbxUVIndices.push_back(UVIndex);
		}
	}

	//std::vector<Luna::Index> tempIndices;

	Luna::Vertex tempVertex; //Temporary vertex
	for (int i = 0; i < vertIndexCount; i++) { //For every vertex
		unsigned int currentVertIndex = vertIndices[i];

		FbxVector4 fbxPos = mesh->GetControlPointAt(currentVertIndex);
		exporter->writer.setVertexPosition(tempVertex, (float)fbxPos[0], (float)fbxPos[1], (float)fbxPos[2]);

		exporter->writer.setVertexNormal(tempVertex, (float)fbxNormals.GetAt(i)[0], (float)fbxNormals.GetAt(i)[1], (float)fbxNormals.GetAt(i)[2]);

		FbxVector2 fbxUV = mesh->GetElementUV()->GetDirectArray().GetAt(fbxUVIndices.at(i));
		exporter->writer.setVertexUV(tempVertex, (float)fbxUV[0], (float)fbxUV[1]);

		FbxVector4 fbxTangent = mesh->GetElementTangent()->GetDirectArray().GetAt(i);
		exporter->writer.setVertexTangent(tempVertex, (float)fbxTangent[0], (float)fbxTangent[1], (float)fbxTangent[2]);

		FbxVector4 fbxBinormal = mesh->GetElementBinormal()->GetDirectArray().GetAt(i);
		exporter->writer.setVertexBiTangent(tempVertex, (float)fbxBinormal[0], (float)fbxBinormal[1], (float)fbxBinormal[2]);

		//bool isDuplicate = false;
		//for (int j = 0; j < tempVertices.size() && !isDuplicate; j++) { //Test for duplicate vertices, this is done as a means of optimization used together with indexed rendering
		//	if (tempVertices.at(j) == tempVertex) { //If the contents of both vertices are the same
		//		isDuplicate = true;

		//		Luna::Index tempIndex;
		//		tempIndex.vertIndex = j;
		//		tempIndices.push_back(tempIndex);
		//	}
		//}
		//if (!isDuplicate) {
		//	tempVertices.push_back(tempVertex);

		//	Luna::Index tempIndex;
		//	tempIndex.vertIndex = (unsigned int)tempVertices.size(); //Current size of array
		//	tempIndices.push_back(tempIndex);
		//}

		tempVertices.push_back(tempVertex);
	}

	tempMesh.vertexCount = (unsigned int)tempVertices.size(); //Set the new vertex count
	exporter->meshVertices.push_back(new Luna::Vertex[tempMesh.vertexCount]); //Creates a new array of vertices for the mesh

	//tempMesh.indexCount = (unsigned int)tempIndices.size(); //Set the new index count
	//exporter->meshIndices.push_back(new Luna::Index[tempMesh.indexCount]); // Creates a new array of indices for the mesh

	//for (unsigned int i = 0; i < tempMesh.indexCount; i++) { //Transfer the index information
	//	exporter->meshIndices[tempMesh.id][i] = tempIndices[i];
	//}

	for (unsigned int i = 0; i < tempMesh.vertexCount; i++) { //Transfer the vertex information
		exporter->meshVertices[tempMesh.id][i] = tempVertices[i];
	}

	GetMaterialData(mesh, exporter); //Since it's a mesh if should have a material which we will get
	tempMesh.hasBoundingBox = GetBoundingBoxData(mesh, exporter);

	exporter->writer.meshes.push_back(tempMesh);
	exporter->writer.scene.meshCount += 1;
}

void Reader::GetMaterialData(FbxMesh* mesh, Exporter* exporter) {
	int matCount = mesh->GetNode()->GetMaterialCount();
	if (matCount > 0) {
		if (matCount != 1) { //Only one material per mesh is supported
			throw("Error: More than one material found in mesh!");
		}
		std::cout << "Material found!" << std::endl; //Debug

		FbxSurfaceMaterial* material = mesh->GetNode()->GetMaterial(0);
		Luna::Material tempMaterial;

		FbxPropertyT<FbxDouble3> fbxProperty;
		if (material->GetClassId().Is(FbxSurfacePhong::ClassId)) { //If it's a Phong material
			std::cout << "The material is Phong." << std::endl; //Debug
			fbxProperty = ((FbxSurfacePhong*)material)->Ambient;
			exporter->writer.setMaterialAmbient(tempMaterial, (float)fbxProperty.Get()[0], (float)fbxProperty.Get()[1], (float)fbxProperty.Get()[2]);
			fbxProperty = ((FbxSurfacePhong*)material)->Diffuse;
			exporter->writer.setMaterialDiffuse(tempMaterial, (float)fbxProperty.Get()[0], (float)fbxProperty.Get()[1], (float)fbxProperty.Get()[2]);
			fbxProperty = ((FbxSurfacePhong*)material)->Specular;
			exporter->writer.setMaterialSpecular(tempMaterial, (float)fbxProperty.Get()[0], (float)fbxProperty.Get()[1], (float)fbxProperty.Get()[2]);
		}

		FbxProperty diffuseProp = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
		unsigned int diffuseMapCount = diffuseProp.GetSrcObjectCount<FbxFileTexture>();
		FbxProperty normalProp = material->FindProperty(FbxSurfaceMaterial::sNormalMap);
		unsigned int normalMapCount = normalProp.GetSrcObjectCount<FbxFileTexture>();
		FbxProperty glowProp = material->FindProperty(FbxSurfaceMaterial::sEmissive);
		unsigned int glowMapCount = glowProp.GetSrcObjectCount<FbxFileTexture>();

		if (diffuseMapCount > 0) {
			const FbxFileTexture* diffTexture = FbxCast<FbxFileTexture>(diffuseProp.GetSrcObject<FbxFileTexture>(0));
			std::experimental::filesystem::path diffTexPath = diffTexture->GetFileName();
			if (std::experimental::filesystem::exists(diffTexPath)) {
				std::experimental::filesystem::copy(diffTexPath, this->outputPath);
			}
			memcpy(tempMaterial.diffuseTexPath, diffTexPath.filename().string().c_str(), PATH_SIZE);
			std::cout << diffTexPath.filename() << std::endl; //Debug
		}

		if (normalMapCount > 0) {
			const FbxFileTexture* normTexture = FbxCast<FbxFileTexture>(normalProp.GetSrcObject<FbxFileTexture>(0));
			std::experimental::filesystem::path normTexPath = normTexture->GetFileName();
			if (std::experimental::filesystem::exists(normTexPath)) {
				std::experimental::filesystem::copy(normTexPath, this->outputPath);
			}
			memcpy(tempMaterial.normalTexPath, normTexPath.filename().string().c_str(), PATH_SIZE);
			std::cout << normTexPath.filename() << std::endl; //Debug
			tempMaterial.hasNormalMap = true;
		}

		if (glowMapCount > 0) {
			const FbxFileTexture* glowTexture = FbxCast<FbxFileTexture>(glowProp.GetSrcObject<FbxFileTexture>(0));
			std::experimental::filesystem::path glowTexPath = glowTexture->GetFileName();
			if (std::experimental::filesystem::exists(glowTexPath)) {
				std::experimental::filesystem::copy(glowTexPath, this->outputPath);
			}
			memcpy(tempMaterial.glowTexPath, glowTexPath.filename().string().c_str(), PATH_SIZE);
			std::cout << glowTexPath.filename() << std::endl; //Debug
			tempMaterial.hasGlowMap = true;
		}

		exporter->writer.materials.push_back(tempMaterial);
		exporter->writer.scene.materialCount++;
	}
}

bool Reader::GetBoundingBoxData(FbxMesh* mesh, Exporter* exporter) {
	for (int i = 0; i < mesh->GetNode()->GetChildCount(); i++) {
		FbxNode* child = mesh->GetNode()->GetChild(i);
		if (isBoundingBox(child)) {
			std::cout << "A bounding box was found!" << std::endl; //Debug

			Luna::BoundingBox tempBBox;

			tempBBox.pos[0] = (float)child->LclTranslation.Get()[0]; //Offsets in case the mesh is not created at origo
			tempBBox.pos[1] = (float)child->LclTranslation.Get()[1];
			tempBBox.pos[2] = (float)child->LclTranslation.Get()[2];

			FbxMesh* bBoxMesh = child->GetMesh();

			unsigned int vertCount = bBoxMesh->GetControlPointsCount();
			
			int minX = 0;
			int minY = 0;
			int minZ = 0;
			int maxX = 0;
			int maxY = 0;
			int maxZ = 0;

			for (int j = 1; j < vertCount; j++) {
				FbxVector4 point = bBoxMesh->GetControlPointAt(j);
				if (point[0] < bBoxMesh->GetControlPointAt(minX)[0])
				{
					minX = j;
				}
				if (point[1] < bBoxMesh->GetControlPointAt(minY)[1])
				{
					minY = j;
				}
				if (point[2] < bBoxMesh->GetControlPointAt(minZ)[2])
				{
					minZ = j;
				}

				if (point[0] > bBoxMesh->GetControlPointAt(maxX)[0])
				{
					maxX = j;
				}
				if (point[1] > bBoxMesh->GetControlPointAt(maxY)[1])
				{
					maxY = j;
				}
				if (point[2] > bBoxMesh->GetControlPointAt(maxZ)[2])
				{
					maxZ = j;
				}
			}

			double max[3];
			double min[3];
			max[0] = bBoxMesh->GetControlPointAt(maxX)[0];
			max[1] = bBoxMesh->GetControlPointAt(maxY)[1];
			max[2] = bBoxMesh->GetControlPointAt(maxZ)[2];
			min[0] = bBoxMesh->GetControlPointAt(minX)[0];
			min[1] = bBoxMesh->GetControlPointAt(minY)[1];
			min[2] = bBoxMesh->GetControlPointAt(minZ)[2];

			tempBBox.halfSize[0] = (float)max[0] / 2;
			tempBBox.halfSize[1] = (float)max[1] / 2;
			tempBBox.halfSize[2] = (float)max[2] / 2;

			exporter->writer.boundingBoxes.push_back(tempBBox);
			return true;
		}
	}
	return false;
}

void Reader::GetSkeletonData(FbxNode* node, Exporter* exporter) {
	exporter->writer.skeletons.resize(exporter->writer.scene.skeletonCount);
}

void Reader::GetWeightsData(FbxMesh* mesh, Exporter* exporter) {
	FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(0, FbxDeformer::eSkin);
	if (skin) {
		unsigned int jointCount = skin->GetClusterCount();
		for (int i = 0; i < jointCount; i++) {
			FbxCluster* cluster = skin->GetCluster(i);
			FbxNode* joint = cluster->GetLink();
			for (int j = 0; j < cluster->GetControlPointIndicesCount(); j++) {
				float weight = cluster->GetControlPointWeights()[j];
			}
		}
	}
}

bool Reader::isBoundingBox(FbxNode* node) {
	char boundingBox[] = "bBox";
	const char* name = node->GetName();

	if (strncmp(boundingBox, name, 4) == 0) { //Compares the first 4 characters of both strings
		return true;
	}
	return false;
}

bool Reader::hasSkeleton(FbxNode* node) {
	FbxMesh* mesh = node->GetMesh();
	FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(0, FbxDeformer::eSkin);
	if (skin != NULL) { //If the mesh is skinned, it has a skeleton
		return true;
	}
	else {
		return false;
	}
}

bool Reader::isTriangulated(FbxMesh* mesh) {
	bool isTriangle = true;
	for (int i = 0; i < mesh->GetPolygonCount() && isTriangle == true; i++) {
		int polySize = mesh->GetPolygonSize(i);
		if (polySize != 3) {
			isTriangle = false;
		}
	}
	return isTriangle;
}
