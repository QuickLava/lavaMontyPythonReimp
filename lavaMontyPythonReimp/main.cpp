#include "lavaMontyPythonReimp.h"

const std::string patchListLocation = ".\\patches.txt";
int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		std::vector<lava::movesetPatch> testPatches;
		std::vector<lava::movesetPatch> patchesFromFile;
		std::vector<std::string> movesetPatches = {};
		std::ifstream movesetPatchesIn(patchListLocation);
		std::string commentChars = "#";
		std::string currentLine = "";
		bool invalidPatches = 0;
		if (movesetPatchesIn.is_open())
		{
			while (std::getline(movesetPatchesIn, currentLine))
			{
				if (currentLine.size() && commentChars.find(currentLine[0]) == std::string::npos)
				{
					if (currentLine.front() == '\"' && currentLine.back() == '\"')
					{
						currentLine = currentLine.substr(1, currentLine.size() - 2);
					}
					std::ifstream patchValidationStream(currentLine);
					if (patchValidationStream.is_open())
					{
						patchValidationStream.close();
						patchesFromFile = lava::parseMovesetPatchXML(currentLine);
						testPatches.insert(testPatches.end(), patchesFromFile.begin(), patchesFromFile.end());
					}
					else
					{
						std::cerr << "[ERROR] Invalid patch file location provided (\"" << currentLine << "\"): the corresponding patches cannot be processed or applied. Please ensure that the provided path is valid and try again.\n\n";
						invalidPatches = 1;
					}
				}
			}
			if (testPatches.size())
			{
				for (int i = 1; i < argc; i++)
				{
					std::string fileIn = argv[i];
					std::cout << fileIn << "... ";
					if (fileIn.rfind(".pac") == fileIn.size() - 4)
					{
						std::cout << "File valid.\n";
						lava::changelogStream.open(fileIn.substr(0, fileIn.rfind('.')) + lava::changelogSuffix);
						lava::changelogStream << "MontyPython Reimp" << "\n";
						lava::changelogStream << "Changelog for " << fileIn.substr(fileIn.rfind('\\') + 1) << "\n\n";
						lava::movesetFile moveset;
						if (moveset.init(fileIn, lava::changelogStream))
						{
							for (std::size_t i = 0; i < testPatches.size(); i++)
							{
								std::cout << "Applying Patch: \"" << testPatches[i].name << "\" (from file: \"" << testPatches[i].sourceFile << "\")\n";
								lava::changelogStream << "Applying Patch: \"" << testPatches[i].name << "\" (from file: \"" << testPatches[i].sourceFile << "\")\n";
								moveset.applyMovesetPatch(testPatches[i], lava::changelogStream);
							}

							std::string outFile = fileIn.substr(0, fileIn.rfind('.'));
							outFile += "_edit.pac";
							moveset.contents.dumpToFile(outFile);
						}
					}
					else
					{
						std::cerr << "File invalid. Ensure that provided files are .pac files.\n";
					}
				}
			}
			else if (invalidPatches)
			{
				std::cerr << "[ERROR] Patch list file (" << patchListLocation << ") was present, but none of the provided files yielded any valid patches. Ensure that your patch files are properly formatted and try again.\n\n";
			}
			else
			{
				std::cerr << "[ERROR] Patch list file (" << patchListLocation << ") was present, but empty. Please specify paths to any patches within that file and try again.\n\n";
			}
		}
	}
	else
	{
		std::cerr << "No target files provided. Must provide valid .pac files to work on. Drag them onto this executable to process them (or specify them as command line arguments).\n";
	}
	std::cout << "Press any key to close this window...\n";
	_getch();
	return 0;
}