#pragma once
#include "Luna/Luna.h"
#include "Luna/Reader.h"
#include <iostream>

class Loader {
public:
	Loader();
	~Loader();

	void PrintData(const char* filePath);

	Luna::Reader reader;

	Luna::Scene scene;
	Luna::Mesh mesh;
	std::vector<Luna::Vertex> vertices;
	std::vector<Luna::Index> indices;
	std::vector<Luna::Material> materials;
private:

};

