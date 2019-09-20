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
	//GetSkeletonData(this->rootNode, exporter);
}

std::string Reader::InputAnimName() {
	std::cout << "Please input a name for the animation: ";
	std::string name;
	std::getline(std::cin, name);
	return name;
}

void Reader::GetData(FbxNode* node, Exporter* exporter) {
	//std::cout << std::endl; //Debug
	//std::cout << node->GetName() << std::endl; //Debug
	//std::cout << node->GetChildCount() << std::endl; //Debug

	for (int i = 0; i < node->GetChildCount(); i++) {
		GetData(node->GetChild(i), exporter); //Recursive
	}
	FbxCamera* camera = node->GetCamera();
	if (camera)
	{
		GetCameraData(node, exporter);
		exporter->writer.scene.hasCamera = true;
	}


	FbxMesh* mesh = node->GetMesh();

	if (mesh && !isBoundingBox(node)) { //If the node is a mesh that is not a bounding box
		GetMeshData(mesh, exporter);
	}

	////Psuedocode
	//if (isCamera) {
	//	//Get camera info
	//}
}

void Reader::GetMeshData(FbxMesh* mesh, Exporter* exporter) {
	//std::cout << "This is a mesh!" << std::endl; //Debug
	Luna::Mesh tempMesh; //Create temporary mesh and fill with info

	memcpy(tempMesh.name, mesh->GetNode()->GetName(), NAME_SIZE);
	tempMesh.id = exporter->writer.scene.meshCount;
	tempMesh.pos[0] = (float)mesh->GetNode()->LclTranslation.Get()[0];
	tempMesh.pos[1] = (float)mesh->GetNode()->LclTranslation.Get()[1];
	tempMesh.pos[2] = (float)mesh->GetNode()->LclTranslation.Get()[2];

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
	if (!isTriangulated(mesh)) {
		Triangulate(mesh, exporter, &tempVertices);
		//throw("Error: The mesh is not triangulated!");
	}
	else {
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

	tempMesh.hasSkeleton = hasSkeleton(mesh->GetNode());
	if (tempMesh.hasSkeleton) {
		//std::cout << "The mesh has a skeleton!" << std::endl; //Debug

		exporter->weights.push_back(new Luna::Weights[tempMesh.vertexCount]); //Create a container for all of the mesh weights
		GetWeightsData(mesh, tempMesh.id, exporter); //Since we know that the mesh is skinned we can get the weights
		GetAnimationData(mesh, tempMesh.id, exporter);
	}

	GetMaterialData(mesh, exporter); //Since it's a mesh it should have a material which we will get
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
		//std::cout << "Material found!" << std::endl; //Debug

		FbxSurfaceMaterial* material = mesh->GetNode()->GetMaterial(0);
		Luna::Material tempMaterial;

		FbxPropertyT<FbxDouble3> fbxProperty;
		if (material->GetClassId().Is(FbxSurfacePhong::ClassId)) { //If it's a Phong material
			//std::cout << "The material is Phong." << std::endl; //Debug
			fbxProperty = ((FbxSurfacePhong*)material)->Ambient;
			exporter->writer.setMaterialAmbient(tempMaterial, (float)fbxProperty.Get()[0], (float)fbxProperty.Get()[1], (float)fbxProperty.Get()[2]);
			fbxProperty = ((FbxSurfacePhong*)material)->Diffuse;
			exporter->writer.setMaterialDiffuse(tempMaterial, (float)fbxProperty.Get()[0], (float)fbxProperty.Get()[1], (float)fbxProperty.Get()[2]);
			fbxProperty = ((FbxSurfacePhong*)material)->Specular;
			exporter->writer.setMaterialSpecular(tempMaterial, (float)fbxProperty.Get()[0], (float)fbxProperty.Get()[1], (float)fbxProperty.Get()[2]);
		}

		FbxProperty diffuseProp = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
		unsigned int diffuseMapCount = diffuseProp.GetSrcObjectCount<FbxFileTexture>();
		FbxProperty specularProp = material->FindProperty(FbxSurfaceMaterial::sSpecular);
		unsigned int specularMapCount = specularProp.GetSrcObjectCount<FbxFileTexture>();
		FbxProperty normalProp = material->FindProperty(FbxSurfaceMaterial::sNormalMap);
		unsigned int normalMapCount = normalProp.GetSrcObjectCount<FbxFileTexture>();
		FbxProperty glowProp = material->FindProperty(FbxSurfaceMaterial::sEmissive);
		unsigned int glowMapCount = glowProp.GetSrcObjectCount<FbxFileTexture>();

		if (diffuseMapCount > 0) {
			const FbxFileTexture* diffTexture = FbxCast<FbxFileTexture>(diffuseProp.GetSrcObject<FbxFileTexture>(0));
			std::experimental::filesystem::path diffTexPath = diffTexture->GetFileName();
			if (std::experimental::filesystem::exists(diffTexPath)) {
				std::experimental::filesystem::copy(diffTexPath, this->outputPath, std::experimental::filesystem::copy_options::overwrite_existing);
			}
			memcpy(tempMaterial.diffuseTexPath, diffTexPath.filename().string().c_str(), PATH_SIZE);
			std::cout << diffTexPath.filename() << std::endl; //Debug
		}

		if (specularMapCount > 0) {
			const FbxFileTexture* specTexture = FbxCast<FbxFileTexture>(specularProp.GetSrcObject<FbxFileTexture>(0));
			std::experimental::filesystem::path specTexPath = specTexture->GetFileName();
			if (std::experimental::filesystem::exists(specTexPath)) {
				std::experimental::filesystem::copy(specTexPath, this->outputPath, std::experimental::filesystem::copy_options::overwrite_existing);
			}
			memcpy(tempMaterial.specularTexPath, specTexPath.filename().string().c_str(), PATH_SIZE);
			std::cout << specTexPath.filename() << std::endl; // Debug
		}

		if (normalMapCount > 0) {
			const FbxFileTexture* normTexture = FbxCast<FbxFileTexture>(normalProp.GetSrcObject<FbxFileTexture>(0));
			std::experimental::filesystem::path normTexPath = normTexture->GetFileName();
			if (std::experimental::filesystem::exists(normTexPath)) {
				std::experimental::filesystem::copy(normTexPath, this->outputPath, std::experimental::filesystem::copy_options::overwrite_existing);
			}
			memcpy(tempMaterial.normalTexPath, normTexPath.filename().string().c_str(), PATH_SIZE);
			std::cout << normTexPath.filename() << std::endl; //Debug
			tempMaterial.hasNormalMap = true;
		}

		if (glowMapCount > 0) {
			const FbxFileTexture* glowTexture = FbxCast<FbxFileTexture>(glowProp.GetSrcObject<FbxFileTexture>(0));
			std::experimental::filesystem::path glowTexPath = glowTexture->GetFileName();
			if (std::experimental::filesystem::exists(glowTexPath)) {
				std::experimental::filesystem::copy(glowTexPath, this->outputPath, std::experimental::filesystem::copy_options::overwrite_existing);
			}
			memcpy(tempMaterial.glowTexPath, glowTexPath.filename().string().c_str(), PATH_SIZE);
			//std::cout << glowTexPath.filename() << std::endl; //Debug
			tempMaterial.hasGlowMap = true;
		}

		exporter->writer.materials.push_back(tempMaterial);
		exporter->writer.scene.materialCount++;
	}
}

bool Reader::GetBoundingBoxData(FbxMesh* mesh, Exporter* exporter) {
	
	Luna::BoundingBox tempBBox;

	for (int i = 0; i < mesh->GetNode()->GetChildCount(); i++) {
		FbxNode* child = mesh->GetNode()->GetChild(i);
		if (isBoundingBox(child)) {
			//std::cout << "A bounding box was found!" << std::endl; //Debug

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

			for (unsigned int j = 1; j < vertCount; j++) {
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

			tempBBox.halfSize[0] = ((float)max[0] - (float)min[0]) / 2;
			tempBBox.halfSize[1] = ((float)max[1] - (float)min[1]) / 2;
			tempBBox.halfSize[2] = ((float)max[2] - (float)min[2]) / 2;

			exporter->writer.boundingBoxes.push_back(tempBBox);
			return true;
		}	
	}

	return false;
}

void Reader::GetWeightsData(FbxMesh* fbxmesh, unsigned int meshID, Exporter* exporter) {
	FbxSkin* skin = (FbxSkin*)fbxmesh->GetDeformer(0, FbxDeformer::eSkin);
	if (skin) {
		unsigned int jointCount = skin->GetClusterCount();
		exporter->writer.skeleton.jointCount = jointCount;
		exporter->joints.resize(jointCount);

		std::vector<SkinData> tempData(fbxmesh->GetControlPointsCount());

		for (unsigned int i = 0; i < jointCount; i++) {
			FbxCluster* cluster = skin->GetCluster(i);
			FbxNode* joint = cluster->GetLink();
			std::string jointName = joint->GetName();
			std::string parentJointName = joint->GetParent()->GetName();

			exporter->joints[i].jointID = i;
			memcpy(exporter->joints[i].jointName, jointName.c_str(), NAME_SIZE);
			exporter->joints[i].parentID = GetJointIdByName(parentJointName.c_str(), exporter, meshID);

			int* indices = cluster->GetControlPointIndices();
			double* weights = cluster->GetControlPointWeights();

			for (int j = 0; j < cluster->GetControlPointIndicesCount(); j++) {
				SkinData& ctrlPnt = tempData[indices[j]];
				if (weights[j] > ctrlPnt.minWeight) {
					ctrlPnt.weight[ctrlPnt.minWeightIndex] = (float)weights[j];
					ctrlPnt.jointID[ctrlPnt.minWeightIndex] = i;

					int minID = 0;
					float newMinWeight = ctrlPnt.weight[minID];
					for (int k = 1; k < 4; k++) { //Find new minimum
						if (ctrlPnt.weight[k] < newMinWeight) {
							minID = k;
							newMinWeight = ctrlPnt.weight[k];
						}
					}
					ctrlPnt.minWeightIndex = minID;
					ctrlPnt.minWeight = newMinWeight;
				}

				float weight = (float)cluster->GetControlPointWeights()[j];
			}
		}

		int vertexId = 0;
		for (int p = 0; p < fbxmesh->GetPolygonCount(); p++)
		{
			for (int v = 0; v < 3; v++)
			{
				int ctrlPointIdx = fbxmesh->GetPolygonVertex(p, v);
				SkinData& data = tempData[ctrlPointIdx];
				unsigned int* joints = data.jointID;
				float* weights = data.weight;
				Luna::Weights& weight = exporter->weights[meshID][vertexId];

				weight.jointIDs[0] = joints[0];
				weight.jointIDs[1] = joints[1];
				weight.jointIDs[2] = joints[2];
				weight.jointIDs[3] = joints[3];

				//Normalizing weights just in case
				float sumWeights  = weights[0] + weights[1] + weights[2] + weights[3];
				weight.weights[0] = weights[0] / sumWeights;
				weight.weights[1] = weights[1] / sumWeights;
				weight.weights[2] = weights[2] / sumWeights;
				weight.weights[3] = weights[3] / sumWeights;
				vertexId++;
			}
		}
	}
}

void Reader::GetAnimationData(FbxMesh* fbxmesh, unsigned int meshID, Exporter* exporter) {
	FbxSkin* skin = (FbxSkin*)fbxmesh->GetDeformer(0, FbxDeformer::eSkin);
	if (skin) {
		std::string animName = InputAnimName();
		memcpy(exporter->writer.animation.animationName, animName.c_str(), NAME_SIZE);

		FbxAnimStack* currStack = this->scene->GetSrcObject<FbxAnimStack>(0); //Get the first animation
		FbxString takeName = currStack->GetName();
		FbxTakeInfo* takeInfo = this->scene->GetTakeInfo(takeName);
		FbxTime::EMode timeMode = this->scene->GetGlobalSettings().GetTimeMode();

		FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
		FbxTime stop = takeInfo->mLocalTimeSpan.GetStop();
		unsigned int start_frame = (unsigned int)start.GetFrameCount(timeMode);
		unsigned int end_frame = (unsigned int)stop.GetFrameCount(timeMode);

		int keyframeCount = (end_frame + 1) - start_frame; // starts from 0
		exporter->writer.animation.duration = (float)takeInfo->mLocalTimeSpan.GetDuration().GetSecondDouble();
		exporter->writer.animation.fps = (float)takeInfo->mLocalTimeSpan.GetDuration().GetFrameRate(timeMode);
		exporter->writer.animation.keyframeCount = keyframeCount;

		int clusterCount = skin->GetClusterCount();
		for (int i = 0; i < clusterCount; i++) {
			FbxCluster* cluster = skin->GetCluster(i);
			exporter->keyframes.push_back(new Luna::Keyframe[keyframeCount]);

			std::string jointName = cluster->GetLink()->GetName();
			// jIndex is the index of THIS joint and matches the index in the skeleton joints array.
			unsigned int jointIndex = 0;
			for (; jointIndex < exporter->writer.skeleton.jointCount; jointIndex++)
			{
				if (jointName.compare(exporter->joints[jointIndex].jointName) == 0){
					break;
				}
			}

			FbxAMatrix geometryTransform(
				fbxmesh->GetNode()->GetGeometricTranslation(FbxNode::eSourcePivot),
				fbxmesh->GetNode()->GetGeometricRotation(FbxNode::eSourcePivot),
				fbxmesh->GetNode()->GetGeometricScaling(FbxNode::eSourcePivot));
			FbxAMatrix meshGlobalTransform; cluster->GetTransformMatrix(meshGlobalTransform);
			FbxAMatrix globalBindPoseTransform; cluster->GetTransformLinkMatrix(globalBindPoseTransform);

			FbxAMatrix invGlobalBindPose = globalBindPoseTransform.Inverse() * meshGlobalTransform * geometryTransform; //Final inverse bindpose matrix
			for (int row = 0; row < 4; row++) { //Transfer to joint
				exporter->joints[jointIndex].invBindposeMatrix[row][0] = (float)invGlobalBindPose[row][0];
				exporter->joints[jointIndex].invBindposeMatrix[row][1] = (float)invGlobalBindPose[row][1];
				exporter->joints[jointIndex].invBindposeMatrix[row][2] = (float)invGlobalBindPose[row][2];
				exporter->joints[jointIndex].invBindposeMatrix[row][3] = (float)invGlobalBindPose[row][3];
			}

			//Keyframes
			for (unsigned int t = start_frame; t < (end_frame + 1); t++) //For all of the keyframes
			{
				Luna::Keyframe& keyframe = exporter->keyframes[i][t];
				FbxTime curr;
				curr.SetFrame(t, timeMode);

				FbxAMatrix currentTransformOffset = fbxmesh->GetNode()->EvaluateGlobalTransform(curr) * geometryTransform;

				FbxAMatrix localTransform = cluster->GetLink()->EvaluateLocalTransform(curr);
				FbxVector4 translation = localTransform.GetT();
				FbxQuaternion rotation = localTransform.GetQ();
				FbxVector4 scale = localTransform.GetS();

				exporter->keyframes[i][t].translation[0] = (float)translation[0];
				exporter->keyframes[i][t].translation[1] = (float)translation[1];
				exporter->keyframes[i][t].translation[2] = (float)translation[2];
				exporter->keyframes[i][t].translation[3] = (float)translation[3];

				exporter->keyframes[i][t].rotation[0] = (float)rotation[0];
				exporter->keyframes[i][t].rotation[1] = (float)rotation[1];
				exporter->keyframes[i][t].rotation[2] = (float)rotation[2];
				exporter->keyframes[i][t].rotation[3] = (float)rotation[3];

				exporter->keyframes[i][t].scale[0] = (float)scale[0];
				exporter->keyframes[i][t].scale[1] = (float)scale[1];
				exporter->keyframes[i][t].scale[2] = (float)scale[2];
				exporter->keyframes[i][t].scale[3] = (float)scale[3];

				exporter->keyframes[i][t].timePosition = (float)t;
			}
		}
	}
}

void Reader::GetCameraData(FbxNode* node, Exporter* exporter)
{
	memcpy(exporter->writer.camera.cameraName, node->GetName(), NAME_SIZE);
	FbxCamera* camera = (FbxCamera*)node->GetNodeAttribute();

	exporter->writer.camera.camPos[0] = camera->Position.Get()[0];
	exporter->writer.camera.camPos[1] = camera->Position.Get()[1];
	exporter->writer.camera.camPos[2] = camera->Position.Get()[2];

	exporter->writer.camera.upVector[0] = camera->UpVector.Get()[0];
	exporter->writer.camera.upVector[1] = camera->UpVector.Get()[1];
	exporter->writer.camera.upVector[2] = camera->UpVector.Get()[2];

	exporter->writer.camera.direction[0] = camera->InterestPosition.Get()[0];
	exporter->writer.camera.direction[1] = camera->InterestPosition.Get()[1];
	exporter->writer.camera.direction[2] = camera->InterestPosition.Get()[2];

	exporter->writer.camera.FoV = camera->FieldOfView.Get();
	exporter->writer.camera.near = camera->NearPlane.Get();
	exporter->writer.camera.far = camera->FarPlane.Get();
}

unsigned int Reader::GetJointIdByName(const char* jointName, Exporter* exporter, unsigned int meshID) {
	for (int i = 0; i < exporter->joints.size(); i++) {
		if (strcmp(exporter->joints[i].jointName, jointName) == 0) {
			return exporter->joints[i].jointID;
		}
	}
	return -1; //If the name isn't found, return -1
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

void Reader::Triangulate(FbxMesh* mesh, Exporter* exporter, std::vector<Luna::Vertex>* vertices)
{
	int polygonCount = mesh->GetPolygonCount();
	std::vector<Luna::Vertex> tempVertices; //Temporary vertex array 
	Luna::Vertex vtx; //Temporary vertex
	int nrOfVtx = 0, vtxIndex = 0;

	for (int i = 0; i < polygonCount; i++)
	{
		int polygonSize = mesh->GetPolygonSize(i);
		for (int j = 0; j < polygonSize; j++)
		{
			int controlPointIndex = mesh->GetPolygonVertex(i, j);
			int textureUVIndex = mesh->GetTextureUVIndex(i, j);
			if (j < 3)
			{
				FbxVector4 fbxPos = mesh->GetControlPointAt(controlPointIndex);
				exporter->writer.setVertexPosition(vtx, (float)fbxPos[0], (float)fbxPos[1], (float)fbxPos[2]);

				FbxVector2 fbxUV = mesh->GetElementUV()->GetDirectArray().GetAt(textureUVIndex);
				exporter->writer.setVertexUV(vtx, (float)fbxUV[0], (float)fbxUV[1]);

				FbxVector4 fbxNormal = mesh->GetElementNormal()->GetDirectArray().GetAt(vtxIndex);
				exporter->writer.setVertexNormal(vtx, (float)fbxNormal[0], (float)fbxNormal[1], (float)fbxNormal[2]);

				FbxVector4 fbxTangent = mesh->GetElementTangent()->GetDirectArray().GetAt(vtxIndex);
				exporter->writer.setVertexTangent(vtx, (float)fbxTangent[0], (float)fbxTangent[1], (float)fbxTangent[2]);

				FbxVector4 fbxBinormal = mesh->GetElementBinormal()->GetDirectArray().GetAt(vtxIndex);
				exporter->writer.setVertexBiTangent(vtx, (float)fbxBinormal[0], (float)fbxBinormal[1], (float)fbxBinormal[2]);
				tempVertices.push_back(vtx);
				nrOfVtx++;
			}
			else
			{
				for (int k = 0; k < 3; k++)
				{
					int replaceJ, addOn;
					switch (k)
					{
					case 0:
						replaceJ = 0;
						addOn = -j;
						break;
					case 1:
						replaceJ = j-1;
						addOn = -1;
						break;
					case 2:
						replaceJ = j;
						addOn = 0;
						break;
					default:
						break;
					}
					controlPointIndex = mesh->GetPolygonVertex(i, replaceJ);
					textureUVIndex = mesh->GetTextureUVIndex(i, replaceJ);

					FbxVector4 fbxPos = mesh->GetControlPointAt(controlPointIndex);
					exporter->writer.setVertexPosition(vtx, (float)fbxPos[0], (float)fbxPos[1], (float)fbxPos[2]);

					FbxVector2 fbxUV = mesh->GetElementUV()->GetDirectArray().GetAt(textureUVIndex);
					exporter->writer.setVertexUV(vtx, (float)fbxUV[0], (float)fbxUV[1]);

					FbxVector4 fbxNormal = mesh->GetElementNormal()->GetDirectArray().GetAt(vtxIndex + addOn);
					exporter->writer.setVertexNormal(vtx, (float)fbxNormal[0], (float)fbxNormal[1], (float)fbxNormal[2]);

					FbxVector4 fbxTangent = mesh->GetElementTangent()->GetDirectArray().GetAt(vtxIndex + addOn);
					exporter->writer.setVertexTangent(vtx, (float)fbxTangent[0], (float)fbxTangent[1], (float)fbxTangent[2]);

					FbxVector4 fbxBinormal = mesh->GetElementBinormal()->GetDirectArray().GetAt(vtxIndex + addOn);
					exporter->writer.setVertexBiTangent(vtx, (float)fbxBinormal[0], (float)fbxBinormal[1], (float)fbxBinormal[2]);
					tempVertices.push_back(vtx);
					nrOfVtx++;
				}
				vtxIndex++;
			}
		}
	}
	*vertices = tempVertices;
}
