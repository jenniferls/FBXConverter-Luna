#include <iostream>
#include <string>
#include <filesystem>

#include "Reader.h"
#include "Exporter.h"
#include "Loader.h"
#include "Luna/Luna.h"

#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)

//#define DELTA 0.0001
//#define EQUAL(A, B) (abs((A)-(B)) < DELTA) ? true:false

enum Commands {
	HELP,
	EXPORT,
	LOAD,
	EXIT,
	NONE
};

void PrintHeader();
void PrintHelp();
void PrintSuccess();
Commands UserInput();

int main() {
	#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	Reader reader;
	bool exit = false;
	Commands cmd = NONE;

	PrintHeader();

	do {
		cmd = UserInput();

		switch (cmd){
			case Commands::HELP: {
				PrintHelp();
				break;
			}
			case Commands::EXPORT: {
				std::string inputFile;
				std::cout << "Please input source FBX-file path: ";
				std::getline(std::cin, inputFile);

				std::experimental::filesystem::path outputPath = inputFile;
				outputPath.replace_extension("lu");

				Reader* reader = new Reader();
				try {
					reader->LoadFBX(inputFile.c_str());
				}
				catch(const char* error){
					std::cout << "Loading " << inputFile << " failed: " << error << std::endl;
					delete reader;
					break;
				}
				Exporter* exporter = new Exporter();
				try {
					reader->ConvertFBX(exporter, outputPath.string().c_str());
					exporter->WriteToBinary(outputPath.string().c_str());
				}
				catch(const char* error) {
					std::cout << "Conversion failed: " << error << std::endl;
					delete reader;
					delete exporter;
					break;
				}

				PrintSuccess();

				delete reader;
				delete exporter;
				break;
			}
			case Commands::LOAD: {
				std::string inputFile;
				std::cout << std::endl << "---------- This option will print information from a Luna-file ----------" << std::endl;
				std::cout << "Please input a source Luna-file path: ";
				std::getline(std::cin, inputFile);

				Loader* loader = new Loader;
				loader->reader.readFile(inputFile.c_str());
				Luna::Mesh mesh;
				std::vector<Luna::Vertex> vertices;
				std::vector<Luna::Index> indices;
				std::vector<Luna::Material> materials;
				Luna::BoundingBox bbox;
				loader->reader.getVertices(0, vertices);
				loader->reader.getIndices(0, indices);
				loader->reader.getMaterials(materials);

				mesh = loader->reader.getMesh(0);
				if (mesh.hasBoundingBox) {
					bbox = loader->reader.getBoundingBox(mesh.id);
				}

				if (mesh.hasSkeleton) {
					Luna::Skeleton skel;
					std::vector<Luna::Weights> weights;
					std::vector<Luna::Joint> joints;
					loader->reader.getWeights(mesh.id, weights);
					loader->reader.getJoints(joints);
				}

				delete loader;
				break;
			}
			case Commands::EXIT: {
				exit = true;
				break;
			}
		}
	} while (exit == false);

	system("pause");
}

void PrintHeader() {

	std::cout << "|-------------------------------------------------------------------------|" << std::endl;
	std::cout << "|----------------- ~ FBX to Luna File Format Converter ~ -----------------|" << std::endl;
	std::cout << "|-------------------------------------------------------------------------|" << std::endl;
	std::cout << std::endl;
}

void PrintHelp() {
	std::cout << std::endl;
	std::cout << "Available Commands:" << std::endl;
	std::cout << "/h, /help " << std::endl;
	std::cout << "/e, /export" << std::endl;
	std::cout << "/l, /load" << std::endl;
	std::cout << "/q, /quit, /exit" << std::endl;
	std::cout << std::endl;
}

void PrintSuccess() {
	std::cout << std::endl;
	std::cout << "|-------------------------------------------------------------------------|" << std::endl;
	std::cout << "|----------------------- ~ Conversion Successful! ~ ----------------------|" << std::endl;
	std::cout << "|-------------------------------------------------------------------------|" << std::endl;
	std::cout << std::endl;
}

Commands UserInput() {
	std::cout << "Please input command: ";
	std::string cmd;
	std::getline(std::cin, cmd);
	if (cmd.compare("/h") == 0 || cmd.compare("/help") == 0) {
		return Commands::HELP;
	}
	if (cmd.compare("/export") == 0 || cmd.compare("/e") == 0) {
		return Commands::EXPORT;
	}
	if (cmd.compare("/load") == 0 || cmd.compare("/l") == 0) {
		return Commands::LOAD;
	}
	if (cmd.compare("/exit") == 0 || cmd.compare("/quit") == 0 || cmd.compare("/q") == 0) {
		return Commands::EXIT;
	}

	std::cout << "The input is not valid. Please type /h for a list of available commands." << std::endl;
	return NONE;
}