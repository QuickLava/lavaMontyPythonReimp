#include "lavaMontyPythonReimp.h"

//#define WIN_DRAG_DROP_FIX
#ifdef WIN_DRAG_DROP_FIX
#include "whereami/whereami.h"
std::string getExecutableDir()
{
	std::string result;
	int length = wai_getModulePath(NULL, 0, NULL);
	char* path = (char*)malloc(length + 1);
	wai_getModulePath(path, length, NULL);
	if (path != nullptr)
	{
		path[length] = '\0';
		result = path;
	}
	return result;
}
#endif

int main(int argc, char* argv[])
{
	std::srand(std::time(0));
	std::string patchListLocation = "patches.txt";
#ifdef WIN_DRAG_DROP_FIX
	std::string executableDir = getExecutableDir();
	executableDir = executableDir.substr(0, executableDir.rfind('\\') + 1);
	patchListLocation = executableDir + patchListLocation;
#endif
	if (argc > 1)
	{
		std::vector<lava::movesetPatch> testPatches;
		std::vector<lava::movesetPatch> patchesFromFile;
		std::vector<std::string> movesetPatches = {};
		std::ifstream movesetPatchesIn;
		std::string commentChars = "#";
		std::string currentLine = "";
		bool invalidPatches = 0;
		movesetPatchesIn.open(patchListLocation);
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
#ifdef WIN_DRAG_DROP_FIX
					if (currentLine.find("./") == 0 || currentLine.find(".\\") == 0)
					{
						currentLine = executableDir + currentLine.substr(2);
					}
#endif
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
						lava::changelogStream.close();
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
		else
		{
			std::cerr << "[ERROR] \""<< patchListLocation << "\" could not be opened! Please create a patch list in the same folder as this executable, and try again.\n";
		}
	}
	else
	{
		std::cerr << "[ERROR] No target files provided. Must provide valid .pac files to work on. Drag them onto this executable to process them (or specify them as command line arguments).\n";
	}
	std::cout << "Press any key to close this window...\n";
	_getch();
	return 0;
}