#include "lavaMontyPythonReimp.h"

namespace lava
{
	const std::vector<char> boundaryTag = { 0x41, 0x52, 0x43 };

	std::string stringToUpper(const std::string& stringIn)
	{
		std::string result = stringIn;
		for (std::size_t i = 0; i < result.size(); i++)
		{
			result[i] = toupper(result[i]);
		}
		return result;
	}
	std::string sanitizeHexStrInput(const std::string& stringIn, bool XAllowed)
	{
		std::string result = stringIn;
		if (result.find("0x") == 0)
		{
			result = result.substr(2, std::string::npos);
		}
		result = lava::stringToUpper(result);
		if (result.size() > 8)
		{
			result = result.substr(result.size() - 8, std::string::npos);
		}
		else if (result.size() < 8)
		{
			result = std::string(8 - result.size(), '0') + result;
		}
		if (!XAllowed)
		{
			for (std::size_t i = 0; i < 8; i++)
			{
				if (result[i] == 'X')
				{
					result[i] = 0;
				}
			}
		}
		return result;
	}



	std::size_t hexVecToNum(const std::vector<char>& vecIn)
	{
		std::size_t result = SIZE_MAX;
		std::size_t vecLength = vecIn.size();
		if (vecLength <= 4)
		{
			int temp = 0x0;
			result = 0;
			for (std::size_t i = 0; i < vecLength; i++)
			{
				result = result << 8;
				temp = vecIn[i];
				temp &= 0x000000FF;
				result |= temp;
			}
		}
		else
		{
			std::cerr << "Provided vector unsupported. Max length is 4 bytes, provided was " << vecLength << ".\n";
		}
		return result;
	}
	std::vector<char> numToHexVec(std::size_t vecIn)
	{
		std::vector<char> result(4, 0);

		union
		{
			std::size_t n;
			char arr[4];
		} s;
		s.n = vecIn;
		result[0] = s.arr[3];
		result[1] = s.arr[2];
		result[2] = s.arr[1];
		result[3] = s.arr[0];
		//std::cout << "Res = " << result << ", src = " << std::hex << vecIn << std::dec << "\n";
		return result;
	}
	std::size_t hexStringToNum(const std::string& stringIn)
	{
		std::size_t result = 0;
		result = std::stoul("0x" + stringIn, nullptr, 16);
		return result;
	}
	std::string numToHexStringWithPadding(std::size_t numIn, std::size_t paddingLength)
	{
		std::stringstream convBuff;
		convBuff << std::hex << numIn;
		std::string result = convBuff.str();
		for (int i = 0; i < result.size(); i++)
		{
			result[i] = std::toupper(result[i]);
		}
		if (result.size() < paddingLength)
		{
			result = std::string(paddingLength - result.size(), '0') + result;
		}
		return result;
	}
	std::string numToDecStringWithPadding(std::size_t numIn, std::size_t paddingLength)
	{
		std::string result = std::to_string(numIn);
		if (result.size() < paddingLength)
		{
			result = std::string(paddingLength - result.size(), '0') + result;
		}
		return result;
	}

	bool hexStrComp(const std::string& str1, const std::string& str2)
	{
		bool result = 0;
		if (str1.size() == str2.size())
		{
			std::size_t len = str1.size();
			std::size_t i = 0;
			result = 1;
			while (result && i < len)
			{
				result = str1[i] == str2[i] || str1[i] == 'X' || str2[i] == 'X' || str1[i] == 'x' || str2[i] == 'x';
				i++;
			}
		}
		return result;
	}
	std::vector<std::pair<std::string, std::string>> createValuePairs(std::ifstream& fileIn)
	{
		std::vector<std::pair<std::string, std::string>> result;
		std::string currentLine = "";
		std::string buff1 = "";
		std::string buff2 = "";
		std::string commentChars = "#";
		std::size_t delimPos = SIZE_MAX;
		while (std::getline(fileIn, currentLine))
		{
			currentLine.erase(std::remove(currentLine.begin(), currentLine.end(), ' '), currentLine.end());
			if (currentLine.size() && commentChars.find(currentLine[0]) == std::string::npos)
			{
				delimPos = currentLine.find(',');
				if (delimPos > 0 && delimPos < (currentLine.size() - 1))
				{
					buff1 = currentLine.substr(0, delimPos).data();
					if (buff1.size() < canonParamLengthStr)
					{
						buff1 = std::string(canonParamLengthStr - buff1.size(), '0') + buff1;
					}
					buff2 = currentLine.substr(delimPos + 1, std::string::npos);
					if (buff2.size() < canonParamLengthStr)
					{
						buff2 = std::string(canonParamLengthStr - buff2.size(), '0') + buff2;
					}
					result.push_back({ buff1, buff2 });
					std::cout << "\tPair: [" << result.back().first << ", " << result.back().second << "]\n";
				}
			}
		}
		return result;
	}

	void addVectors(std::vector<std::size_t>& vec1, const std::vector<std::size_t>& vec2)
	{
		std::size_t minSize = std::min(vec1.size(), vec2.size());
		for (std::size_t i = 0; i < minSize; i++)
		{
			vec1[i] += vec2[i];
		}
	}

	std::vector<movesetPatch> parseMovesetPatchXML(std::string fileIn)
	{
		std::vector<movesetPatch> result;
		pugi::xml_document patchXMLIn;
		pugi::xml_parse_result parseRes = patchXMLIn.load_file(fileIn.c_str());
		if (parseRes.status == pugi::xml_parse_status::status_ok)
		{
			for (pugi::xml_node patch = patchXMLIn.first_child(); patch; patch = patch.next_sibling())
			{
				movesetPatch currPatch;
				for (pugi::xml_attribute patchAttr = patch.first_attribute(); patchAttr; patchAttr = patchAttr.next_attribute())
				{
					if (patchAttr.name() == "name")
					{
						currPatch.name = patchAttr.as_string();
					}
				}
				for (pugi::xml_node patchChild = patch.first_child(); patchChild; patchChild = patchChild.next_sibling())
				{
					if (patchChild.name() == "targets")
					{
						for (pugi::xml_node patchTarget = patchChild.first_child(); patchTarget; patchTarget = patchTarget.next_sibling())
						{
							movesetPatchTarget currTarget;
							for (pugi::xml_attribute targetAttr = patchTarget.first_attribute(); targetAttr; targetAttr = targetAttr.next_attribute())
							{
								if (targetAttr.name() == "name")
								{
									currTarget.name = targetAttr.as_string();
								}
								else if (targetAttr.name() == "signature")
								{
									currTarget.signature = lava::numToHexVec(lava::hexStringToNum(lava::sanitizeHexStrInput(targetAttr.as_string(), 0)));
								}
								else if (targetAttr.name() == "paramIndex")
								{
									std::string manipStr = targetAttr.as_string();
									if (manipStr.find("0x") == 0)
									{
										manipStr = manipStr.substr(2, std::string::npos);
										currTarget.paramIndex = std::stoi(manipStr, nullptr, 16);
									}
									else
									{
										currTarget.paramIndex = targetAttr.as_int(INT_MAX);
										if (currTarget.paramIndex < 0)
										{
											currTarget.paramIndex = INT_MAX;
										}
									}
								}
							}
							if (currTarget.paramIndex != INT_MAX)
							{
								currPatch.targets.push_back(currTarget);
							}
							else
							{
								std::cerr << "[ERROR] Error in parsing param index for Target \"" << currTarget.name << "\" in moveset patch XML (" << fileIn << ").\n Ensure that this value is a positive integer value.\n" ;
							}
						}
					}
					else if (patchChild.name() == "modifications")
					{
						for (pugi::xml_node patchModification = patchChild.first_child(); patchModification; patchModification = patchModification.next_sibling())
						{
							movesetPatchMod currMod;
							for (pugi::xml_attribute patchModAttr = patchModification.first_attribute(); patchModAttr; patchModAttr = patchModAttr.next_attribute())
							{
								if (patchModAttr.name() == "match")
								{
									currMod.match = lava::sanitizeHexStrInput(patchModAttr.as_string(), 1);
								}
								else if (patchModAttr.name() == "lock")
								{
									currMod.locked = lava::sanitizeHexStrInput(patchModAttr.as_string(), 0);
								}
								else if (patchModAttr.name() == "paramIndexRedirect")
								{
									std::string manipStr = patchModAttr.as_string();
									if (manipStr.find("0x") == 0)
									{
										manipStr = manipStr.substr(2, std::string::npos);
										currMod.redirect = std::stoi(manipStr, nullptr, 16);
									}
									else
									{
										currMod.redirect = patchModAttr.as_int(INT_MAX);
										if (currMod.redirect < 0)
										{
											currMod.redirect = INT_MAX;
										}
									}
								}
							}
							for (pugi::xml_node modAction = patchModification.first_child(); modAction; modAction = modAction.next_sibling())
							{
								movesetPatchModAction currAction;
								for (pugi::xml_attribute actionAttr = modAction.first_attribute(); actionAttr; actionAttr = actionAttr.next_attribute())
								{
									if (actionAttr.name() == "type")
									{
										currAction.actionType = actionAttr.as_int();
									}
									else if (actionAttr.name() == "value")
									{
										currAction.value = lava::sanitizeHexStrInput(actionAttr.as_string(), 0);
									}
								}
								currMod.actions.push_back(currAction);
							}
							currPatch.modifications.push_back(currMod);
						}
					}
				}
				result.push_back(currPatch);
			}
		}
		else
		{
			std::cerr << "Failed to parse moveset patch XML file: " << parseRes.description() << "\n";
		}
		return result;
	}



	/*movesetPatch lava::createMovesetPatch(std::ifstream& fileIn)
	{
		movesetPatch result;
		if (fileIn.is_open())
		{
			std::string currentLine = "";
			char token = 0;
			while (std::getline(fileIn, currentLine))
			{
				if ()
			}
		}
		return result;
	}*/



	/*std::vector<char> stringToVec(const std::string& stringIn)
	{
		return(std::vector<char>(stringIn.begin(), stringIn.end()));
	}
	std::string vecToString(const std::vector<char>& vecIn)
	{
		return(std::string(vecIn.begin(), vecIn.end()));
	}*/

	void byteArray::populate(std::istream& sourceStream)
	{
		std::string buffer = "";
		sourceStream.seekg(0, sourceStream.end);
		std::size_t sourceSize(sourceStream.tellg());
		sourceStream.seekg(0, sourceStream.beg);
		body.resize(sourceSize);
		sourceStream.read(body.data(), sourceSize);
		size = body.size();
		std::cout << "Populated array with " << size << " byte(s) of data.\n";
	}
	std::vector<char> byteArray::getBytes(std::size_t numToGet, std::size_t startIndex, std::size_t& numGotten)
	{
		numGotten = 0;
		if (startIndex < size)
		{
			if (startIndex + numToGet > size)
			{
				numToGet = size - startIndex;
				return std::vector<char>(body.begin() + startIndex, body.end());
			}
			numGotten = numToGet;
			return std::vector<char>(body.begin() + startIndex, body.begin() + startIndex + numToGet);
		}
		else
		{
			std::cerr << "Requested region startpoint was invalid. Specified index was [" << startIndex << "], max valid index is [" << size - 1 << "].\n";
		}
		return std::vector<char>();
	}
	bool byteArray::setBytes(std::vector<char> bytesIn, std::size_t atIndex)
	{
		bool result = 0;
		if ((atIndex + bytesIn.size()) > atIndex && atIndex + bytesIn.size() < size)
		{
			int tempInt = 0;
			char* tempPtr = body.data() + atIndex;
			/*std::cout << std::hex;
			for (int i = 0; i < bytesIn.size(); i++)
			{
				tempInt = *(tempPtr + i);
				tempInt &= 0x000000FF;
				std::cout << ((tempInt < 0x10) ? "0" : "") << tempInt;
			}
			std::cout << "\n" << std::dec;*/
			std::memcpy(body.data() + atIndex, bytesIn.data(), bytesIn.size());
			/*std::cout << std::hex;
			for (int i = 0; i < bytesIn.size(); i++)
			{
				tempInt = *(tempPtr + i);
				tempInt &= 0x000000FF;
				std::cout << ((tempInt < 0x10) ? "0" : "") << tempInt;
			}
			std::cout << "\n" << std::dec;*/
			result = 1;
		}
		return result;
	}
	bool byteArray::dumpToFile(std::string targetPath)
	{
		bool result = 0;
		std::ofstream output;
		output.open(targetPath, std::ios_base::binary | std::ios_base::out);
		if (output.is_open())
		{
			output.write(body.data(), size);
			result = 1;
			std::cout << "Dumped body to \"" << targetPath << "\".\n";
		}
		return result;
	}
	std::size_t byteArray::search(const std::vector<char>& searchCriteria, std::size_t startItr, std::size_t endItr)
	{
		std::vector<char>::iterator itr = body.end();
		if (endItr < startItr)
		{
			endItr = SIZE_MAX;
		}
		if (endItr > size)
		{
			endItr = size;
		}
		if (size && startItr < size && searchCriteria.size())
		{
			itr = std::search(body.begin() + startItr, body.begin() + endItr, searchCriteria.begin(), searchCriteria.end());
		}
		return (itr != body.end()) ? itr - body.begin() : SIZE_MAX;
	}
	std::vector<std::size_t> byteArray::searchMultiple(const std::vector<char>& searchCriteria, std::size_t startItr, std::size_t endItr)
	{
		std::size_t cursor = startItr;
		std::vector<std::size_t> result;
		std::size_t critSize = searchCriteria.size();
		bool done = 0;
		while (!done && cursor <= endItr)
		{
			cursor = search(searchCriteria, cursor);
			if (cursor != SIZE_MAX )
			{
				result.push_back(cursor);
				if ((cursor + critSize) > cursor)
				{
					cursor += critSize;
				}
			}
			else
			{
				done = 1;
			}
		}
		return result;
	}


	bool movesetFile::init(std::string filePathIn)
	{
		std::ifstream input;
		input.open(filePathIn, std::ios_base::binary | std::ios_base::in);
		if (input.is_open())
		{
			contents.populate(input);
			if (contents.size)
			{
				filePath = filePathIn;
				std::size_t numGotten;
				std::cout << "Header Info:\n" << std::hex;
				movesetLength = lava::hexVecToNum(contents.getBytes(4, 0x60, numGotten));
				dataLength = lava::hexVecToNum(contents.getBytes(4, 0x64, numGotten));
				dataOffset = 0x80;
				std::cout << "\tMoveset Section[0x80]: Total Length = 0x" << movesetLength << ", Data Length = 0x" << dataLength << "\n";
				offsetSectionCount = lava::hexVecToNum(contents.getBytes(4, 0x68, numGotten));
				offsetSectionOffset = dataOffset + dataLength;
				std::cout << "\tOffset Section[0x" << offsetSectionOffset << "]: Entry Count = 0x" << offsetSectionCount << "\n";
				dataTableCount = lava::hexVecToNum(contents.getBytes(4, 0x6C, numGotten));
				dataTableOffset = offsetSectionOffset + (offsetSectionCount * 0x4);
				std::cout << "\tData Table Section[0x" << dataTableOffset << "]: Entry Count = 0x" << dataTableCount << "\n";
				externalDataCount = lava::hexVecToNum(contents.getBytes(4, 0x70, numGotten));
				externalDataOffset = dataTableOffset + (dataTableCount * 0x8);
				std::cout << "\tExternal Data Table Section[0x" << externalDataOffset << "]: Entry Count = 0x" << externalDataCount << "\n";
				tablesEnd = externalDataOffset + (externalDataCount * 0x8);
				std::cout << "\tTables End[0x" << tablesEnd << "]";
				std::cout << "\n" << std::dec;
				std::cout << "\tData Table Contents:\n";
				summarizeTable(dataTableOffset, dataTableCount, 0x80, "\t\t");
				std::cout << "\tExternal Data Table Contents:\n";
				summarizeTable(externalDataOffset, externalDataCount, 0x0, "\t\t");

				std::ofstream logOut;
				logOut.open(filePathIn.substr(0, filePathIn.rfind('.')) + changelogSuffix, std::ios_base::app);
				if (logOut.is_open())
				{
					logOut << "Header Info:\n" << std::hex;
					logOut << "\tMoveset Section[0x80]: Total Length = 0x" << movesetLength << ", Data Length = 0x" << dataLength << "\n";
					logOut << "\tOffset Section[0x" << offsetSectionOffset << "]: Entry Count = 0x" << offsetSectionCount << "\n";
					logOut << "\tData Table Section[0x" << dataTableOffset << "]: Entry Count = 0x" << dataTableCount << "\n";
					logOut << "\tExternal Data Table Section[0x" << externalDataOffset << "]: Entry Count = 0x" << externalDataCount << "\n";
					logOut << "\tTables End[0x" << tablesEnd << "]";
					logOut << "\n\n" << std::dec;
					
					//logOut << "\tData Table Contents:\n";
					//summarizeTable(dataTableOffset, dataTableCount, 0x80, "\t\t", logOut);
					//logOut << "\tExternal Data Table Contents:\n";
					//summarizeTable(externalDataOffset, externalDataCount, 0x0, "\t\t", logOut);
					//logOut << "\n";
				}
			}
		}
		return contents.size > 0;
	}
	std::string movesetFile::fetchString(std::size_t strAddr)
	{
		std::string result = "";
		std::size_t endAddr = contents.search({0x00}, strAddr);
		if (endAddr != SIZE_MAX)
		{
			result = std::string(contents.body.begin() + strAddr, contents.body.begin() + endAddr);
		}
		return result;
	}
	void movesetFile::summarizeTable(std::size_t tableAddr, std::size_t entryCount, std::size_t offsetShiftSize, std::string prefix, std::ostream& output)
	{
		if (tableAddr < contents.size)
		{
			output << std::hex;
			std::size_t addrCap = tableAddr + (entryCount * 0x8);
			std::size_t numGotten;
			std::string temp;
			output << prefix << "[Associated String]" << std::string(64 - 19, ' ') << "[Address]" << "\n";
			for (std::size_t cursorAddr = tableAddr; cursorAddr < addrCap; cursorAddr += 0x8)
			{
				temp = fetchString(lava::hexVecToNum(contents.getBytes(0x4, cursorAddr + 0x4, numGotten)) + tablesEnd);
				output << prefix << temp << std::string(64 - temp.size(), ' ') << "0x" << lava::hexVecToNum(contents.getBytes(4, cursorAddr, numGotten)) + offsetShiftSize << "\n";
			}
			output << std::dec;
		}
	}

	std::vector<std::size_t> movesetFile::changeMatchingParameter(std::vector<std::pair<std::string, std::vector<char>>> functions, std::vector<std::pair<std::string, std::string>> matches, std::size_t parameterOffsetInBytes)
	{
		std::vector<std::size_t> matchResults;
		matchResults.resize(matches.size(), 0);
		std::size_t affected = 0;
		std::size_t funcCount = functions.size();
		std::size_t pingCount = 0;
		std::size_t matchesCount = matches.size();
		bool matchFound = 0;
		std::string paramValStr = "";
		std::pair<std::string, std::vector<char>>* currFunc = nullptr;
		std::vector<std::size_t> searchPings = {};
		std::vector<char> paramOffset = {};
		std::size_t paramOffsetNum = 0;
		std::vector<char> paramVal = {};
		std::ofstream logOut;
		logOut.open(filePath.substr(0, filePath.rfind('.')) + changelogSuffix, std::ios_base::app);
		for (std::size_t i = 0; i < funcCount; i++)
		{
			currFunc = &functions[i];
			logOut << currFunc->first << " Param Changes:\n";
			std::cout << "Processing Function \"" << currFunc->first << "\":\n";
			searchPings = contents.searchMultiple(currFunc->second, 0, movesetLength + 0x60 - currFunc->second.size());
			pingCount = searchPings.size();
			std::size_t numGotten;
			for (std::size_t u = 0; u < pingCount; u++)
			{
				paramOffset = contents.getBytes(4, searchPings[u] + currFunc->second.size(), numGotten);
				if (numGotten == 4)
				{
					paramOffsetNum = hexVecToNum(paramOffset);
					std::cout << "\t[0x" << std::hex << searchPings[u] << "]Params @ 0x" << paramOffsetNum << " (0x";
					paramOffsetNum += globalBaseOffset;
					std::cout << paramOffsetNum << " for abs. offset)" << std::dec << "\n";
					paramVal = contents.getBytes(canonParamLengthInBytes, paramOffsetNum + parameterOffsetInBytes, numGotten);
					std::cout << "\t\tParam Val: " << paramVal << "\n";
					std::size_t y = 0;
					matchFound = 0;
					while (!matchFound && y < matchesCount)
					{
						paramValStr = numToHexStringWithPadding(hexVecToNum(paramVal), 8);
						std::pair<std::string, std::string>* matchPtr = &matches[y];
						if (hexStrComp(matchPtr->first, paramValStr))
						{
							matchResults[y]++;
							logOut << "\t[0x" << std::hex << lava::numToHexStringWithPadding(searchPings[u], 8) << std::dec << "->0x" << paramOffset << "]: Changed Param from 0x" << paramValStr << " to 0x";
							std::cout << "\t\tFOUND MATCH! ";
							matchFound = 1;
							for (std::size_t t = 0; t < canonParamLengthStr; t++)
							{
								if (matchPtr->second[t] != 'x' && matchPtr->second[t] != 'X')
								{
									paramValStr[t] = matchPtr->second[t];
								}
							}
							contents.setBytes(numToHexVec(hexStringToNum(paramValStr)), paramOffsetNum + parameterOffsetInBytes);
							logOut << contents.getBytes(canonParamLengthInBytes, paramOffsetNum + parameterOffsetInBytes, numGotten) << "\n";
							std::cout << "Param Transformed to: " << contents.getBytes(canonParamLengthInBytes, paramOffsetNum + parameterOffsetInBytes, numGotten) << "\n";
							affected++;
						}
						y++;
					}
					//std::cout << "\t\tBody Preview:\n";
					//for (std::size_t y = 0; y < 4; y++)
					//{
					//	std::cout << "\t\t\t" << contents.getBytes(32, paramOffsetNum + (32 * y), numGotten) << "\n";
					//}
				}
				else
				{
					std::cerr << "\tSomething went wrong, received wrong number of bytes for param offset while processing " << currFunc->first << " instance @ 0x" << std::hex << searchPings[u] << std::dec << ".\n";
				}
			}
			std::cout << "\n";
		}

		std::cout << "Match summary:\n";
		logOut << "Match summary:\n";
		for (std::size_t i = 0; i < matchesCount; i++)
		{
			if (matchResults[i] == 0)
			{
				std::cout << "\tMatch #" << numToDecStringWithPadding(i, 3) << " (" << matches[i].first << " -> " << matches[i].second << ") was never used.\n";
				logOut << "\tMatch #" << numToDecStringWithPadding(i, 3) << " (" << matches[i].first << " -> " << matches[i].second <<") was never used.\n";
			}
			else
			{
				std::cout << "\tMatch #" << numToDecStringWithPadding(i, 3) << " (" << matches[i].first << " -> " << matches[i].second << ") used " << matchResults[i] << " time(s).\n";
				logOut << "\tMatch #" << numToDecStringWithPadding(i, 3) << " (" << matches[i].first << " -> " << matches[i].second << ") used " << matchResults[i] << " time(s).\n";
			}
		}
		std::cout << "\n";
		logOut << "\n";

		return matchResults;
	}

	lava::movesetPatchResult movesetFile::applyMovesetPatch(const lava::movesetPatch& patchIn)
	{
		// Initialize results struct
		lava::movesetPatchResult results;
		// Store sizes of respective lists
		std::size_t targetCount = patchIn.targets.size();
		std::size_t modificationCount = patchIn.modifications.size();
		// Resize result list
		results.timesTargetsHit.resize(targetCount, std::vector<std::size_t>(modificationCount, 0));
		// Initialize changelog output
		std::ofstream logOut;
		logOut.open(filePath.substr(0, filePath.rfind('.')) + changelogSuffix, std::ios_base::app);

		// Format float precision for output streams
		std::cout << std::fixed;
		std::cout << std::setprecision(3);
		logOut << std::fixed;
		logOut << std::setprecision(3);

		// Initialize variables for reuse in the loop
		std::size_t numGotten = 0;
		std::vector<std::size_t> searchPings = {};

		std::vector<char> paramOffset;
		paramOffset.resize(4, 0);
		std::vector<char> paramTypeIdentifier;
		paramTypeIdentifier.resize(4, 0);
		std::size_t paramTypeIDNum = 0;
		std::vector<char> paramVal;
		paramVal.resize(4, 0);
		std::string paramValString;
		paramValString.resize(8, 0);
		unsigned int paramValNum = 0;
		

		// Loop through every target
		for (std::size_t targetItr = 0; targetItr < targetCount; targetItr++)
		{
			// Grab pointer to current target, and ping for its signature within the data section
			const movesetPatchTarget* currTarget = &patchIn.targets[targetItr];
			std::cout << "\"" << currTarget->name << "\" [0x" << currTarget->signature << "] (Param " << currTarget->paramIndex << "):\n";
			logOut << "\"" << currTarget->name << "\" [0x" << currTarget->signature << "] (Param " << currTarget->paramIndex << "):\n";

			searchPings = contents.searchMultiple(currTarget->signature, dataOffset, dataOffset + dataLength);

			// Loop through every received ping
			for (std::size_t pingItr = 0; pingItr < searchPings.size(); pingItr++)
			{
				// Receive and validate that ping's parameter offset
				std::size_t currPing = searchPings[pingItr];
				std::vector<char> paramOffset = contents.getBytes(4, currPing + currTarget->signature.size(), numGotten);
				if (numGotten == 4)
				{
					// Convert that offset to a size_t, and apply dataOffset (makes offset relative to beginning of .PAC file instead of beginning of moveset section
					std::size_t paramOffsetNum = lava::hexVecToNum(paramOffset) + dataOffset;
					// Report location of current ping, as well as its paramOffset. Note, the first paramOffset we report is non .pac adjusted since that's the one you'll see in PSAC, which is what I expect most people will be looking for. The adjusted value is provided second.
					logOut << "\t[0x" << numToHexStringWithPadding(currPing, 8) << "]: Params @ 0x" << numToHexStringWithPadding(paramOffsetNum - dataOffset, 8) << " (0x" << numToHexStringWithPadding(paramOffsetNum, 0) << " for .pac relative offset)" << "\n";
					std::cout << "\t[0x" << numToHexStringWithPadding(currPing, 8) << "]: Params @ 0x" << numToHexStringWithPadding(paramOffsetNum - dataOffset, 8) << " (0x" << numToHexStringWithPadding(paramOffsetNum, 0) << " for .pac relative offset)" << "\n";

					// Get the value of the parameter at the targeted index, and create string rep. Note that because all parameters are 8 bytes long (4 bytes for type identifier, 4 for value), getting the value of parameter i means we look ((8*i) + 4) bytes in relative to the param offset.
					std::size_t targetParamIndexOffset = 0;
					std::string intermediateParamValString = "";

					auto setTargetParam = [&](int targetParam)
					{
						if (paramOffsetNum != SIZE_MAX && targetParam >= 0)
						{
							targetParamIndexOffset = ((8 * targetParam) + 4);
							paramVal = contents.getBytes(4, paramOffsetNum + targetParamIndexOffset, numGotten);
							paramValString = numToHexStringWithPadding(hexVecToNum(paramVal), 8);
							intermediateParamValString = paramValString;
							paramValNum = hexStringToNum(paramValString);
							// Get the value of the parameter's type identifier. This ensures that variables are manipulated in the expected way.
							paramTypeIdentifier = contents.getBytes(4, paramOffsetNum + targetParamIndexOffset - 4, numGotten);
							paramTypeIDNum = lava::hexVecToNum(paramTypeIdentifier);
						}
					};
					auto saveTargetParam = [&]()
					{
						if (paramOffsetNum != SIZE_MAX)
						{
							contents.setBytes(numToHexVec(hexStringToNum(paramValString)), paramOffsetNum + targetParamIndexOffset);
							contents.setBytes(numToHexVec(paramTypeIDNum), paramOffsetNum + targetParamIndexOffset - 4);
						}
					};

					setTargetParam(currTarget->paramIndex);

					//paramVal = contents.getBytes(4, paramOffsetNum + targetParamIndexOffset, numGotten);
					//paramValString = numToHexStringWithPadding(hexVecToNum(paramVal), 8);
					//paramValNum = hexStringToNum(paramValString);

					//// Get the value of the parameter's type identifier. This ensures that variables are manipulated in the expected way.
					//paramTypeIdentifier = contents.getBytes(4, paramOffsetNum + targetParamIndexOffset - 4, numGotten);
					//paramTypeIDNum = lava::hexVecToNum(paramTypeIdentifier);

					// Report value
					logOut << "\t\tParam Val: " << paramVal;
					std::cout << "\t\tParam Val: " << paramVal;
					if (paramTypeIDNum == lava::movesetVarTypes::varTy_SCLR)
					{
						logOut << " (Scalar = " << paramValNum / lava::floatDenominator << ")";
						std::cout << " (Scalar = " << paramValNum / lava::floatDenominator << ")";
					}
					// Initialize values for loop
					std::size_t modItr = 0;
					bool matchFound = 0;
					bool doScalarActionPrint = 0;
					bool doScalarFinalPrint = 0;
					const movesetPatchMod* currMod = nullptr;

					for (std::size_t modItr = 0; modItr < modificationCount; modItr++)
					{
						// Record pointer to current modification
						currMod = &patchIn.modifications[modItr];

						// Handle redirect
						if (currMod->redirect != INT_MAX)
						{
							setTargetParam(currMod->redirect);
							/*targetParamIndexOffset = ((8 * currMod->redirect) + 4);
							paramVal = contents.getBytes(4, paramOffsetNum + targetParamIndexOffset, numGotten);
							paramValString = numToHexStringWithPadding(hexVecToNum(paramVal), 8);
							paramValNum = hexStringToNum(paramValString);

							paramTypeIdentifier = contents.getBytes(4, paramOffsetNum + targetParamIndexOffset - 4, numGotten);
							paramTypeIDNum = lava::hexVecToNum(paramTypeIdentifier);*/
							logOut << "\t\tParameter Redirect Triggered: New target is Param Index " << currMod->redirect << "\n";
							logOut << "\t\tParam Val: " << paramValString;
							std::cout << "\t\tParameter Redirect Triggered: New target is Param Index " << currMod->redirect << "\n";
							std::cout << "\t\tParam Val: " << paramValString;
							if (paramTypeIDNum == lava::movesetVarTypes::varTy_SCLR)
							{
								logOut << " (Scalar = " << paramValNum / lava::floatDenominator << ")";
								std::cout << " (Scalar = " << paramValNum / lava::floatDenominator << ")";
							}
							logOut << " (Redirected)\n";
							std::cout << " (Redirected)\n";
						}

						// If the current mod's match value matches the paramValue string
						if (hexStrComp(currMod->match, paramValString))
						{
							// Record that the modification was used and report it to the console.
							matchFound = 1;
							results.timesTargetsHit[targetItr][modItr]++;

							logOut << "\n\t\tFOUND MATCH!\n";
							logOut << "\t\tModification #" << modItr << ":\n";
							std::cout << "\n\t\tFOUND MATCH!\n";
							std::cout << "\t\tModification #" << modItr << ":\n";

							// Initialize variables for use in loop
							const movesetPatchModAction* currAction = nullptr;
							const std::string canonParamValString = paramValString;
							std::string intermediateParamValString = paramValString;
							bool actionOccured = 0;

							// Iterate through each action:
							for (std::size_t actionItr = 0; actionItr < currMod->actions.size(); actionItr++)
							{
								// Store intermediate paramString
								intermediateParamValString = paramValString;
								// Reset action tracker
								actionOccured = 0;
								doScalarActionPrint = 0;
								doScalarFinalPrint = 0;

								// Do changelog reporting
								logOut << "\t\t\tAction #" << actionItr << ": ";
								std::cout << "\t\t\tAction #" << actionItr << ": ";
								currAction = &currMod->actions[actionItr];
								// Perform the appropriate changes based on the type of action
								switch (currAction->actionType)
								{
									case modActionTypes::actTy_DO_NOTHING:
									{
										logOut << "[NOTHING]";
										std::cout << "[NOTHING]";
										break;
									}
									case modActionTypes::actTy_REPLACE:
									{
										actionOccured = 1;
										for (std::size_t i = 0; i < canonParamLengthStr; i++)
										{
											if (currAction->value[i] != 'X')
											{
												paramValString[i] = currAction->value[i];
											}
										}
										logOut << "[REP]";
										std::cout << "[REP]";
										break;
									}
									case modActionTypes::actTy_ADD:
									{
										actionOccured = 1;
										unsigned int incomingValNum = hexStringToNum(currAction->value);
										paramValNum += incomingValNum;
										paramValString = numToHexStringWithPadding(paramValNum, 8);
										logOut << "[ADD]";
										std::cout << "[ADD]";
										break;
									}
									case modActionTypes::actTy_SUB:
									{
										actionOccured = 1;
										unsigned int incomingValNum = hexStringToNum(currAction->value);
										paramValNum -= incomingValNum;
										paramValString = numToHexStringWithPadding(paramValNum, 8);
										logOut << "[SUB]";
										std::cout << "[SUB]";
										break;
									}
									case modActionTypes::actTy_MUL:
									{
										actionOccured = 1;
										doScalarActionPrint = 1;
										//float incomingValNum = hexStringToNum(currAction->value) / lava::floatDenominator;
										unsigned int incomingValNum = hexStringToNum(currAction->value);
										paramValNum *= incomingValNum;
										paramValString = numToHexStringWithPadding(paramValNum, 8);
										logOut << "[MUL]";
										std::cout << "[MUL]";
										break;
									}
									case modActionTypes::actTy_DIV:
									{
										actionOccured = 1;
										doScalarActionPrint = 1;
										//float incomingValNum = hexStringToNum(currAction->value) / lava::floatDenominator;
										unsigned int incomingValNum = hexStringToNum(currAction->value);
										paramValNum /= incomingValNum;
										paramValString = numToHexStringWithPadding(paramValNum, 8);
										logOut << "[DIV]";
										std::cout << "[DIV]";
										break;
									}
									case modActionTypes::actTy_BIT_AND:
									{
										actionOccured = 1;
										paramValNum &= hexStringToNum(currAction->value);
										paramValString = numToHexStringWithPadding(paramValNum, 8);
										logOut << "[BIT_AND]";
										std::cout << "[BIT_AND]";
										break;
									}
									case modActionTypes::actTy_BIT_OR:
									{
										actionOccured = 1;
										paramValNum |= hexStringToNum(currAction->value);
										paramValString = numToHexStringWithPadding(paramValNum, 8);
										logOut << "[BIT_OR]";
										std::cout << "[BIT_OR]";
										break;
									}
									case modActionTypes::actTy_BIT_XOR:
									{
										actionOccured = 1;
										paramValNum ^= hexStringToNum(currAction->value);
										paramValString = numToHexStringWithPadding(paramValNum, 8);
										logOut << "[BIT_XOR]";
										std::cout << "[BIT_XOR]";
										break;
									}
									case modActionTypes::actTy_RETARGET_PARAM:
									{
										if (hexStringToNum(currAction->value) >= 0)
										{
											//actionOccured = 1;
											saveTargetParam();
											setTargetParam(hexStringToNum(currAction->value));
											logOut << "[TARGET_PARAM]\n";
											std::cout << "[TARGET_PARAM]\n";
											logOut << "\t\t\tParameter Redirect Triggered: New target is Param Index " << hexStringToNum(currAction->value) << "\n";
											logOut << "\t\t\tParam Val: " << paramValString;
											std::cout << "\t\t\tParameter Redirect Triggered: New target is Param Index " << hexStringToNum(currAction->value) << "\n";
											std::cout << "\t\t\tParam Val: " << paramValString;
											if (paramTypeIDNum == lava::movesetVarTypes::varTy_SCLR)
											{
												logOut << " (Scalar = " << paramValNum / lava::floatDenominator << ")";
												std::cout << " (Scalar = " << paramValNum / lava::floatDenominator << ")";
											}
											logOut << " (Redirected)\n";
											std::cout << " (Redirected)\n";
										}
										break;
									}
									default:
									{
										break;
									}
								}

								if (actionOccured)
								{
									if (paramTypeIDNum == lava::movesetVarTypes::varTy_SCLR || doScalarActionPrint)
									{
										logOut << " Value = " << currAction->value;
										std::cout << " Value = " << currAction->value;
										float incomingFlt = hexStringToNum(currAction->value) / lava::floatDenominator;
										logOut << " (Scalar = " << incomingFlt << ")";
										std::cout << " (Scalar = " << incomingFlt << ")";
										float intermediateFlt = lava::hexStringToNum(intermediateParamValString) / lava::floatDenominator;
										logOut << "\n\t\t\t\t" << intermediateParamValString << " (" << intermediateFlt << ") -> " << paramValString << " (" << paramValNum / lava::floatDenominator << ")\n";
										std::cout << "\n\t\t\t\t" << intermediateParamValString << " (" << intermediateFlt << ") -> " << paramValString << " (" << paramValNum / lava::floatDenominator << ")\n";
									}
									else
									{
										logOut << " Value = " << currAction->value << "\n\t\t\t\t" << intermediateParamValString << " -> " << paramValString << "\n";
										std::cout << " Value = " << currAction->value << "\n\t\t\t\t" << intermediateParamValString << " -> " << paramValString << "\n";
									}
								}

								// Update paramValNum representation
								paramValNum = lava::hexStringToNum(paramValString);
							}
							bool lockUsed = 0;
							for (std::size_t i = 0; i < 8; i++)
							{
								if (currMod->locked[i] == '1' && paramValString[i] != canonParamValString[i])
								{
									paramValString[i] = canonParamValString[i];
									lockUsed = 1;
								}
							}
							if (lockUsed)
							{
								std::cout << "\t\t\tLock (" << currMod->locked << "): " << paramValString << "\n";
								logOut << "\t\t\tLock (" << currMod->locked << "): " << paramValString << "\n";

								// Update paramValNum representation
								paramValNum = lava::hexStringToNum(paramValString);
							}

							// Write result to contents.
							saveTargetParam();
							//contents.setBytes(numToHexVec(hexStringToNum(paramValString)), paramOffsetNum + targetParamIndexOffset);

							// Report results:
							logOut << "\t\t\tFinal Value: " << contents.getBytes(4, paramOffsetNum + targetParamIndexOffset, numGotten);
							std::cout << "\t\t\tFinal Value: " << contents.getBytes(4, paramOffsetNum + targetParamIndexOffset, numGotten);
							// Do scalar report if necessary
							if (paramTypeIDNum == lava::movesetVarTypes::varTy_SCLR || doScalarFinalPrint)
							{
								logOut << " (Scalar = " << paramValNum / lava::floatDenominator << ")";
								std::cout << " (Scalar = " << paramValNum / lava::floatDenominator << ")";
							}
							logOut << "\n";
							std::cout << "\n";
						}
						// Advance mod itr
					}
					if (!matchFound)
					{
						logOut << "\n";
						std::cout << "\n";
					}
				}
				else
				{
					std::cerr << "\tSomething went wrong, received wrong number of bytes for param offset while processing " << currTarget->name << " instance @ 0x" << std::hex << currPing << std::dec << ".\n";
				}
			}
		}

		// Do summary
		std::cout << "Patch Results Summary:\n";
		logOut << "Patch Results Summary:\n";
		// Iterate through each target:
		for (std::size_t i = 0; i < targetCount; i++)
		{
			logOut << "\t\"" << patchIn.targets[i].name << "\":\n";
			std::cout << "\t\"" << patchIn.targets[i].name << "\":\n";
			for (std::size_t modItr = 0; modItr < modificationCount; modItr++)
			{
				logOut << "\t\tModification #" << numToDecStringWithPadding(modItr, 2);
				std::cout << "\t\tModification #" << numToDecStringWithPadding(modItr, 2);
				std::size_t appliedCount = results.timesTargetsHit[i][modItr];
				if (appliedCount == 0)
				{
					logOut << " was never applied.\n";
					std::cout << " was never applied.\n";
				}
				else
				{
					logOut << " applied " << appliedCount << " time(s)\n";
					std::cout << " applied " << appliedCount << " time(s)\n";
				}
			}
		}
		std::cout << "\n";
		logOut << "\n";

		return results;
	}



}

//std::ostream& operator<<(std::ostream& out, const std::vector<char>& in)
//{
//	out << std::hex;
//	std::size_t size = in.size();
//	int val = 0;
//	for (int i = 0; i < size; i++)
//	{
//		val = in[i];
//		val &= 0x000000FF;
//		out << ((val < 0x10) ? "0": "") << val;
//	}
//	out << std::dec;
//	return out;
//}

std::ostream& operator<<(std::ostream& out, const std::vector<char>& in)
{
	if (in.size())
	{
		int val = lava::hexVecToNum(in);
		std::string res = lava::numToHexStringWithPadding(val, in.size() * 2);
		out << res;
	}
	return out;
}
