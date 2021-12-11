#include "lavaMontyPythonReimp.h"

//std::vector<std::pair<std::string, std::vector<char>>> soundCommands =
//{
//	{"Sound Effect",					{0x0A, 0x00, 0x01, 0x00}},
//	{"Sound Effect 2",					{0x0A, 0x01, 0x01, 0x00}},
//	{"Stop Sound Effect",				{0x0A, 0x03, 0x01, 0x00}},
//	{"Sound Effect (Transient)",		{0x0A, 0x02, 0x01, 0x00}},
//	{"Other Sound Effect (landing)",	{0x0A, 0x09, 0x01, 0x00}},
//	{"Other Sound Effect (impact)",		{0x0A, 0x0A, 0x01, 0x00}},
//	{"Victory",							{0x0A, 0x05, 0x01, 0x00}},
//	{"Stepping Sound Effect",			{0x0A, 0x01, 0x01, 0x00}}
//};
//std::vector<std::pair<std::string, std::vector<char>>> graphicsCommands =
//{
//	{"Graphic Effect",					{0x11, 0x00, 0x10, 0x00}},
//	{"Graphic Effect (Attached)" ,		{0x11, 0x01, 0x0A, 0x00}},
//	{"Graphic Effect (Attached)(+100)" ,{0x11, 0x01, 0x0B, 0x00}},
//	{"Graphic Effect (Attached 2)" ,	{0x11, 0x02, 0x0A, 0x00}},
//	{"Graphic Effect (Stepping)" ,		{0x11, 0x1A, 0x10, 0x00}},
//	{"Graphic Effect (Landing)" ,		{0x11, 0x1B, 0x10, 0x00}},
//	{"Graphic Effect (Tumbling)" ,		{0x11, 0x1C, 0x10, 0x00}},
//	{"Graphic Effect (Attached 19)" ,	{0x11, 0x19, 0x0A, 0x00}},
//	{"Terminate Graphic Effect" ,		{0x11, 0x15, 0x03, 0x00}}
//};
//std::vector<std::pair<std::string, std::vector<char>>> traceCommands =
//{
//	{"Sword Glow",						{0x11, 0x03, 0x14, 0x00}},
//	{"Sword/Hammer Glow",				{0x11, 0x04, 0x17, 0x00}},
//	//{"Terminate Sword Glow",			{0x11, 0x05, 0x01, 0x00}}
//};

int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		std::string fileIn = argv[1];
		std::cout << fileIn << "\n";
		if (fileIn.rfind(".pac") == fileIn.size() - 4)
		{
			std::cout << "File valid.\n";
			std::ofstream logOut;
			logOut.open( fileIn.substr(0, fileIn.rfind('.')) + lava::changelogSuffix);
			logOut << "MontyPython Reimp" << "\n";
			logOut << "Changelog for " << fileIn.substr(fileIn.rfind('\\') + 1) << "\n\n";
			logOut.close();
			lava::movesetFile moveset;
			if (moveset.init(fileIn))
			{
				logOut.open(fileIn.substr(0, fileIn.rfind('.')) + lava::changelogSuffix, std::ios_base::app);

				std::ifstream movesetPatchIn;
				movesetPatchIn.open("neo_mapping_format.xml");
				if (movesetPatchIn.is_open())
				{
					movesetPatchIn.close();
					std::vector<lava::movesetPatch> testPatches = lava::parseMovesetPatchXML("neo_mapping_format.xml");
					for (std::size_t i = 0; i < testPatches.size(); i++)
					{
						std::cout << "Applying Patch: \"" << testPatches[i].name << "\"\n";
						logOut << "Applying Patch: \"" << testPatches[i].name << "\"\n";
						logOut.close();
						moveset.applyMovesetPatch(testPatches[i]);
						logOut.open(fileIn.substr(0, fileIn.rfind('.')) + lava::changelogSuffix, std::ios_base::app);
					}
				}
				std::string outFile = fileIn.substr(0, fileIn.rfind('.'));
				outFile += "_edit.pac";
				moveset.contents.dumpToFile(outFile);

				//std::ifstream gfxIDIn;
				//gfxIDIn.open(".\\file_mapping.txt");
				//std::vector<std::pair<std::string, std::string>> gfxIDPairs;
				//std::ifstream sfxIDIn;
				//sfxIDIn.open(".\\sound_mapping.txt");
				//std::vector<std::pair<std::string, std::string>> sfxIDPairs;
				//std::ifstream traceIDIn;
				//traceIDIn.open(".\\trace_mapping.txt");
				//std::vector<std::pair<std::string, std::string>> traceIDPairs;
				//if (gfxIDIn.is_open())
				//{
				//	std::cout << "Collecting GFX Pairs:\n";
				//	gfxIDPairs = lava::createValuePairs(gfxIDIn);
				//	logOut << "GFX Value Pairs:\n";
				//	for (int i = 0; i < gfxIDPairs.size(); i++)
				//	{
				//		logOut << "\t[" << gfxIDPairs[i].first << ", " << gfxIDPairs[i].second << "]\n";
				//	}
				//}
				//if (sfxIDIn.is_open())
				//{
				//	std::cout << "Collecting SFX Pairs:\n";
				//	sfxIDPairs = lava::createValuePairs(sfxIDIn);
				//	logOut << "SFX Value Pairs:\n";
				//	for (int i = 0; i < sfxIDPairs.size(); i++)
				//	{
				//		logOut << "\t[" << sfxIDPairs[i].first << ", " << sfxIDPairs[i].second << "]\n";
				//	}
				//}
				//if (traceIDIn.is_open())
				//{
				//	std::cout << "Collecting Trace Pairs:\n";
				//	traceIDPairs = lava::createValuePairs(traceIDIn);
				//	logOut << "Trace ID Value Pairs:\n";
				//	for (int i = 0; i < traceIDPairs.size(); i++)
				//	{
				//		logOut << "\t[" << traceIDPairs[i].first << ", " << traceIDPairs[i].second << "]\n";
				//	}
				//}
				//logOut << "\nNote: To get absolute param addresses add 0x" << std::hex << lava::globalBaseOffset << std::dec << " to recorded offset.\n\n";
				//logOut.close();
				//
				//if (!gfxIDPairs.empty())
				//{
				//	moveset.changeMatchingParameter(graphicsCommands, gfxIDPairs, 0x4);
				//	moveset.changeMatchingParameter(traceCommands, gfxIDPairs, 0x5C);
				//}
				//if (!sfxIDPairs.empty())
				//{
				//	moveset.changeMatchingParameter(soundCommands, sfxIDPairs, 0x4);
				//}
				//if (!traceIDPairs.empty())
				//{
				//	moveset.changeMatchingParameter(traceCommands, traceIDPairs, 0x4);
				//}
			}
		}
		/*else if (fileIn.rfind(".txt") == fileIn.size() - 4)
		{
			std::ifstream temp;
			temp.open(fileIn);
			if (temp.is_open())
			{
				std::ofstream tempOut;
				std::size_t adjustmentSize = (0xA5 * 7);
				std::size_t origVal;
				tempOut.open(fileIn.substr(0, fileIn.rfind(".")) + "_adjust.txt" );
				tempOut << "Shifting Values from " << fileIn << " by 0x" << std::hex << adjustmentSize << std::dec << ".\n";
				tempOut << "\t[Orig]\t[Mod]\n" << std::hex;
				std::string currentLine;
				while (std::getline(temp, currentLine))
				{
					origVal = lava::hexStringToNum(currentLine);
					tempOut << "\t" << currentLine << "\t" << lava::numToHexStringWithPadding(origVal + adjustmentSize, 8) << "\n";
				}
				tempOut << std::dec;
			}
		}*/
	}
	else
	{
		std::cerr << "No file provided. Must provide a file to work on. Drag file onto this executable to process it.\n";
	}
	std::cout << "Press any key to close this window...\n";
	_getch();
	return 0;
}