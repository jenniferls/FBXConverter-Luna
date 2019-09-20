#pragma once
#include "../Luna/Luna.h"
#include "../Luna/LunaReader.h"
#include <iostream>

class FBXLoader {
public:
	FBXLoader();
	~FBXLoader();

	void PrintData(const char* filePath);

	Luna::LunaReader reader;

	Luna::Scene scene;
	Luna::Mesh mesh;
	std::vector<Luna::Vertex> vertices;
	std::vector<Luna::Index> indices;
	std::vector<Luna::Material> materials;
private:

};

