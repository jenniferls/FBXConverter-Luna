#pragma once
#include "Luna/Luna.h"
#include "Luna/Writer.h"
#include <fstream>
#include <iostream>
#include <vector>

class FBXExporter {
public:
	FBXExporter();
	~FBXExporter();

	Luna::Writer getWriter() const;

	void WriteToBinary(const char* filename);

	Luna::Writer writer;

	std::vector<Luna::Vertex*> meshVertices;
	std::vector<Luna::Index*> meshIndices;
	std::vector<Luna::Joint> joints;
	std::vector<Luna::Weights*> weights;
	std::vector<Luna::Keyframe*> keyframes;

private:
	
};