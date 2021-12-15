#include "lavaMontyPythonReimp.h"

namespace lava
{
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
								else if (targetAttr.name() == "paramType")
								{
									currTarget.paramIndex = targetAttr.as_int(INT_MAX);
									if (currTarget.paramIndex < 0)
									{
										currTarget.paramIndex = INT_MAX;
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
								else if (patchModAttr.name() == "extraCondition")
								{
									std::string manipStr = patchModAttr.as_string();
									if (manipStr.find("0x") == 0)
									{
										manipStr = manipStr.substr(2, std::string::npos);
										currMod.extraCondition = std::stoi(manipStr, nullptr, 16);
									}
									else
									{
										currMod.extraCondition = patchModAttr.as_int(INT_MAX);
										if (currMod.extraCondition < 0)
										{
											currMod.extraCondition = lava::extraConditionTypes::exCon_NULL;
										}
									}
								}
								else if (patchModAttr.name() == "evalMethod")
								{
									std::string manipStr = patchModAttr.as_string();
									if (manipStr.find("0x") == 0)
									{
										manipStr = manipStr.substr(2, std::string::npos);
										currMod.matchMethod = std::stoi(manipStr, nullptr, 16);
									}
									else
									{
										currMod.matchMethod = patchModAttr.as_int(INT_MAX);
										if (currMod.matchMethod < 0)
										{
											currMod.matchMethod = lava::matchEvaluationMethod::mtEvl_EQUALS;
										}
									}
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
										currMod.paramIndexRedirect = std::stoi(manipStr, nullptr, 16);
									}
									else
									{
										currMod.paramIndexRedirect = patchModAttr.as_int(INT_MAX);
										if (currMod.paramIndexRedirect < 0)
										{
											currMod.paramIndexRedirect = INT_MAX;
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
										std::string manipStr = actionAttr.as_string();
										if (manipStr.find("0x") == 0)
										{
											manipStr = manipStr.substr(2, std::string::npos);
											currAction.actionType = std::stoi(manipStr, nullptr, 16);
										}
										else
										{
											currAction.actionType = actionAttr.as_int(INT_MAX);
											if (currAction.actionType < 0)
											{
												currAction.actionType = INT_MAX;
											}
										}
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
			std::cerr << "\nRequested region startpoint was invalid. Specified index was [" << startIndex << "], max valid index is [" << size - 1 << "].\n";
		}
		return std::vector<char>();
	}
	bool byteArray::setBytes(std::vector<char> bytesIn, std::size_t atIndex)
	{
		bool result = 0;
		if ((atIndex + bytesIn.size()) > atIndex && atIndex + bytesIn.size() < size)
		{
			/*int tempInt = 0;
			char* tempPtr = body.data() + atIndex;
			std::cout << "Original Value: " << std::hex;
			for (int i = 0; i < bytesIn.size(); i++)
			{
				tempInt = *(tempPtr + i);
				tempInt &= 0x000000FF;
				std::cout << ((tempInt < 0x10) ? "0" : "") << tempInt;
			}
			std::cout << "\n" << std::dec;*/
			std::memcpy(body.data() + atIndex, bytesIn.data(), bytesIn.size());
			/*tempInt = 0;
			tempPtr = body.data() + atIndex;
			std::cout << "Modified Value: " << std::hex;
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
	void movesetFile::summarizeOffsetSection(std::ostream& output, std::size_t adjustment)
	{
		std::size_t numGotten = 0;
		std::size_t maxOffset = offsetSectionOffset + offsetSectionCount * 4;
		std::vector<char> offset = { 0, 0, 0, 0 };
		std::map<std::size_t, std::vector<char>> contentsPartitioned;
		std::ofstream trace(filePath.substr(0, filePath.rfind('.')) + "_offset.txt");
		for (std::size_t cursor = offsetSectionOffset; cursor < maxOffset; cursor += 4)
		{
			offset = contents.getBytes(4, cursor, numGotten);
			if (numGotten == 4)
			{
				contentsPartitioned.insert(std::make_pair<std::size_t, std::vector<char>>(lava::hexVecToNum(offset) + adjustment, {}));
				trace << std::hex << lava::hexVecToNum(offset) + adjustment << std::dec << "\n";
			}
		}
		std::map<std::size_t, std::vector<char>>::iterator endItr = contentsPartitioned.end();
		std::map<std::size_t, std::vector<char>>::iterator prevItr = contentsPartitioned.begin();
		std::map<std::size_t, std::vector<char>>::iterator itr = prevItr;
		itr++;
		for (itr; itr != endItr; itr++)
		{
			prevItr->second = contents.getBytes(itr->first - prevItr->first, prevItr->first, numGotten);
			if (numGotten != (itr->first - prevItr->first))
			{
				std::cerr << "INVALID READ\n";
			}
			prevItr++;
		}
		endItr = contentsPartitioned.end();
		prevItr = contentsPartitioned.begin();
		itr = prevItr;
		itr++;
		std::size_t index = 0;
		for (itr; itr != endItr; itr++)
		{
			output << index << "-" << std::hex << prevItr->first - adjustment << std::dec << "-" << itr->first - prevItr->first;
			output << std::string((16 - (output.tellp() % 16)), '-');
			output.write((char*)prevItr->second.data(), prevItr->second.size() * sizeof(char));
			output << std::string((16 - (output.tellp() % 16)), '-');
			index++;
			prevItr++;
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

		// Loop through every target
		for (std::size_t targetItr = 0; targetItr < targetCount; targetItr++)
		{
			// Grab pointer to current target, and ping for its signature within the data section
			const movesetPatchTarget* currTarget = &patchIn.targets[targetItr];
			std::cout << "\"" << currTarget->name << "\" [0x" << currTarget->signature << "] (Param " << currTarget->paramIndex;
			logOut << "\"" << currTarget->name << "\" [0x" << currTarget->signature << "] (Param " << currTarget->paramIndex;
			if (currTarget->paramType != INT_MAX)
			{
				std::cout << ", of Type " << currTarget->paramType;
				logOut << ", of Type " << currTarget->paramType;
			}
			std::cout << "):\n";
			logOut << "):\n";
			
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

					// Create our param target. This holds the current value of the target parameter, as well as where it's stored in the file, and is the variable that we'll be modifying to make changes to the parameter's value and type.
					lava::paramTarget neoParamVals(*this, paramOffsetNum, currTarget->paramIndex);

					if (currTarget->paramType == INT_MAX || neoParamVals.getParamTypeNum() == currTarget->paramType)
					{
						// Report location of current ping, as well as its paramOffset. Note, the first paramOffset we report is non .pac adjusted since that's the one you'll see in PSAC, which is what I expect most people will be looking for. The adjusted value is provided second.
						logOut << "\t[0x" << numToHexStringWithPadding(currPing, 8) << "]: Params @ 0x" << numToHexStringWithPadding(paramOffsetNum - dataOffset, 8);
						std::cout << "\t[0x" << numToHexStringWithPadding(currPing, 8) << "]: Params @ 0x" << numToHexStringWithPadding(paramOffsetNum - dataOffset, 8);

						// Initialize values for loop
						bool redirectUsed = 0;
						bool previousModUsed = 0;
						std::size_t modItr = 0;
						bool matchFound = 0;
						bool doScalarActionPrint = 0;
						bool doScalarFinalPrint = 0;
						const movesetPatchMod* currMod = nullptr;

						logOut << ", Param Val: " << neoParamVals.getParamValue();
						std::cout << ", Param Val: " << neoParamVals.getParamValue();
						if (neoParamVals.getParamTypeNum() == lava::movesetParamTypes::varTy_SCLR)
						{
							logOut << " (Scalar = " << neoParamVals.getParamValueNum() / lava::floatDenominator << ")";
							std::cout << " (Scalar = " << neoParamVals.getParamValueNum() / lava::floatDenominator << ")";
						}
						logOut << "\n";
						std::cout << "\n";

						for (std::size_t modItr = 0; modItr < modificationCount; modItr++)
						{
							// Record pointer to current modification
							currMod = &patchIn.modifications[modItr];

							// Handle redirect
							if (currMod->paramIndexRedirect != INT_MAX)
							{
								neoParamVals = lava::paramTarget(*this, paramOffsetNum, currMod->paramIndexRedirect);

								redirectUsed = 1;
							}
							else
							{
								neoParamVals = lava::paramTarget(*this, paramOffsetNum, currTarget->paramIndex);

								redirectUsed = 0;
							}

							bool matchEvalRes = 0;
							std::string tempEvalStr = currMod->match;
							for (std::size_t i = 0; i < tempEvalStr.size(); i++)
							{
								if (tempEvalStr[i] == 'X')
								{
									tempEvalStr[i] = neoParamVals.getParamValueString()[i];
								}
							}
							switch (currMod->matchMethod)
							{
								case lava::matchEvaluationMethod::mtEvl_EQUALS:
								{
									matchEvalRes = hexStrComp(tempEvalStr, neoParamVals.getParamValueString());
									break;
								}
								case lava::matchEvaluationMethod::mtEvl_NOT_EQUALS:
								{
									matchEvalRes = !hexStrComp(tempEvalStr, neoParamVals.getParamValueString());
									break;
								}
								case lava::matchEvaluationMethod::mtEvl_GREATER:
								{
									matchEvalRes = lava::hexStringToNum(tempEvalStr) > neoParamVals.getParamValueNum();
									break;
								}
								case lava::matchEvaluationMethod::mtEvl_GREATER_OE:
								{
									matchEvalRes = lava::hexStringToNum(tempEvalStr) >= neoParamVals.getParamValueNum();
									break;
								}
								case lava::matchEvaluationMethod::mtEvl_LESSER:
								{
									matchEvalRes = lava::hexStringToNum(tempEvalStr) < neoParamVals.getParamValueNum();
									break;
								}
								case lava::matchEvaluationMethod::mtEvl_LESSER_OE:
								{
									matchEvalRes = lava::hexStringToNum(tempEvalStr) <= neoParamVals.getParamValueNum();
									break;
								}
								case lava::matchEvaluationMethod::mtEvl_BIT_AND:
								{
									matchEvalRes = lava::hexStringToNum(tempEvalStr) & neoParamVals.getParamValueNum();
									break;
								}
								case lava::matchEvaluationMethod::mtEvl_BIT_XOR:
								{
									matchEvalRes = lava::hexStringToNum(tempEvalStr) ^ neoParamVals.getParamValueNum();
									break;
								}
								default:
								{
									matchEvalRes = 0;
									break;
								}
							}

							bool extraConditionRes = 0;
							switch (currMod->extraCondition)
							{
								case lava::extraConditionTypes::exCon_PREV_USED:
								{
									extraConditionRes = previousModUsed;
									break;
								}
								case lava::extraConditionTypes::exCon_PREV_NOT_USED:
								{
									extraConditionRes = !previousModUsed;
									break;
								}
								default:
								{
									extraConditionRes = 1;
									break;
								}
							}

							// If the current mod's match value matches the paramValue string
							if (matchEvalRes && extraConditionRes)
							{
								// Record that the modification was used and report it to the console.
								matchFound = 1;
								previousModUsed = 1;
								results.timesTargetsHit[targetItr][modItr]++;

								logOut << "\t\tModification #" << modItr << ":\n";
								std::cout << "\t\tModification #" << modItr << ":\n";
								if (redirectUsed)
								{
									logOut << "\t\t(Redirect Triggered, New Target Index is " << currMod->paramIndexRedirect << ")\n";
									std::cout << "\t\t(Redirect Triggered, New Target Index is " << currMod->paramIndexRedirect << ")\n";
								}

								if (1 || currMod->matchMethod)
								{
									logOut << "\t\tFound match (Match:" << currMod->match << " ";
									std::cout << "\t\tFound match (Match:" << currMod->match << " ";
									switch (currMod->matchMethod)
									{
										case lava::matchEvaluationMethod::mtEvl_EQUALS:
										{
											std::cout << "==";
											logOut << "==";
											break;
										}
										case lava::matchEvaluationMethod::mtEvl_NOT_EQUALS:
										{
											std::cout << "!=";
											logOut << "!=";
											break;
										}
										case lava::matchEvaluationMethod::mtEvl_GREATER:
										{
											std::cout << ">";
											logOut << ">";
											break;
										}
										case lava::matchEvaluationMethod::mtEvl_GREATER_OE:
										{
											std::cout << ">=";
											logOut << ">=";
											break;
										}
										case lava::matchEvaluationMethod::mtEvl_LESSER:
										{
											std::cout << "<";
											logOut << "<";
											break;
										}
										case lava::matchEvaluationMethod::mtEvl_LESSER_OE:
										{
											std::cout << "<=";
											logOut << "<=";
											break;
										}
										case lava::matchEvaluationMethod::mtEvl_BIT_AND:
										{
											std::cout << "&";
											logOut << "&";
											break;
										}
										case lava::matchEvaluationMethod::mtEvl_BIT_XOR:
										{
											std::cout << "^";
											logOut << "^";
											break;
										}
										default:
										{
											break;
										}
									}
									logOut << " Param Value:" << neoParamVals.getParamValueString() << ")\n";
									std::cout << " Param Value:" << neoParamVals.getParamValueString() << ")\n";
								}
								else
								{
									logOut << "\t\tFound match (Pattern: " << currMod->match << ", Param Value: " << neoParamVals.getParamValueString() << ")\n";
									std::cout << "\t\tFound match (Pattern: " << currMod->match << ", Param Value: " << neoParamVals.getParamValueString() << ")\n";
								}

								if (currMod->extraCondition)
								{
									std::cout << "\t\tExtra Condition also met:";
									logOut << "\t\tExtra Condition also met:";
									switch (currMod->extraCondition)
									{
									case lava::extraConditionTypes::exCon_PREV_USED:
									{
										std::cout << " [REQ_PREV]\n";
										logOut << " [REQ_PREV]\n";
										break;
									}
									case lava::extraConditionTypes::exCon_PREV_NOT_USED:
									{
										std::cout << " [REQ_NOT_PREV]\n";
										logOut << " [REQ_NOT_PREV]\n";
										break;
									}
									default:
									{
										break;
									}
									}
								}

								// Initialize variables for use in loop
								const movesetPatchModAction* currAction = nullptr;
								const std::string canonParamValString = neoParamVals.getParamValueString();
								std::string intermediateParamValString = neoParamVals.getParamValueString();
								bool actionOccured = 0;

								// Iterate through each action:
								for (std::size_t actionItr = 0; actionItr < currMod->actions.size(); actionItr++)
								{
									// Store intermediate paramString
									intermediateParamValString = neoParamVals.getParamValueString();
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
											logOut << "[NOP]";
											std::cout << "[NOP]";
											break;
										}
										case modActionTypes::actTy_REPLACE:
										{
											actionOccured = 1;

											std::string manipStr = neoParamVals.getParamValueString();
											for (std::size_t i = 0; i < canonParamLengthStr; i++)
											{
												if (currAction->value[i] != 'X')
												{
													manipStr[i] = currAction->value[i];
												}
											}
											neoParamVals.updateParamValue(manipStr);

											logOut << "[REP]";
											std::cout << "[REP]";
											break;
										}
										case modActionTypes::actTy_INT_ADD:
										{
											actionOccured = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = neoParamVals.getParamValueNum();
											manipNum += incomingValNum;
											neoParamVals.updateParamValue(manipNum);

											logOut << "[INT_ADD]";
											std::cout << "[INT_ADD]";
											break;
										}
										case modActionTypes::actTy_INT_SUB:
										{
											actionOccured = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = neoParamVals.getParamValueNum();
											manipNum -= incomingValNum;
											neoParamVals.updateParamValue(manipNum);

											logOut << "[INT_SUB]";
											std::cout << "[INT_SUB]";
											break;
										}
										case modActionTypes::actTy_INT_MUL:
										{
											actionOccured = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = neoParamVals.getParamValueNum();
											manipNum *= incomingValNum;
											neoParamVals.updateParamValue(manipNum);

											logOut << "[INT_MUL]";
											std::cout << "[INT_MUL]";
											break;
										}
										case modActionTypes::actTy_INT_DIV:
										{
											actionOccured = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = neoParamVals.getParamValueNum();
											manipNum /= incomingValNum;
											neoParamVals.updateParamValue(manipNum);

											logOut << "[INT_DIV]";
											std::cout << "[INT_DIV]";
											break;
										}
										case modActionTypes::actTy_FLT_ADD:
										{
											actionOccured = 1;
											doScalarActionPrint = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = neoParamVals.getParamValueNum();
											manipNum += incomingValNum;
											neoParamVals.updateParamValue(manipNum);

											logOut << "[FLT_ADD]";
											std::cout << "[FLT_ADD]";
											break;
										}
										case modActionTypes::actTy_FLT_SUB:
										{
											actionOccured = 1;
											doScalarActionPrint = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = neoParamVals.getParamValueNum();
											manipNum -= incomingValNum;
											neoParamVals.updateParamValue(manipNum);

											logOut << "[FLT_SUB]";
											std::cout << "[FLT_SUB]";
											break;
										}
										case modActionTypes::actTy_FLT_MUL:
										{
											actionOccured = 1;
											doScalarActionPrint = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = neoParamVals.getParamValueNum();
											manipNum *= incomingValNum;
											neoParamVals.updateParamValue(manipNum);

											logOut << "[FLT_MUL]";
											std::cout << "[FLT_MUL]";
											break;
										}
										case modActionTypes::actTy_FLT_DIV:
										{
											actionOccured = 1;
											doScalarActionPrint = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = neoParamVals.getParamValueNum();
											manipNum /= incomingValNum;
											neoParamVals.updateParamValue(manipNum);

											logOut << "[FLT_DIV]";
											std::cout << "[FLT_DIV]";
											break;
										}
										case modActionTypes::actTy_BIT_AND:
										{
											actionOccured = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = neoParamVals.getParamValueNum();
											manipNum &= incomingValNum;
											neoParamVals.updateParamValue(manipNum);

											logOut << "[BIT_AND]";
											std::cout << "[BIT_AND]";
											break;
										}
										case modActionTypes::actTy_BIT_OR:
										{
											actionOccured = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = neoParamVals.getParamValueNum();
											manipNum |= incomingValNum;
											neoParamVals.updateParamValue(manipNum);

											logOut << "[BIT_OR]";
											std::cout << "[BIT_OR]";
											break;
										}
										case modActionTypes::actTy_BIT_XOR:
										{
											actionOccured = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = neoParamVals.getParamValueNum();
											manipNum ^= incomingValNum;
											neoParamVals.updateParamValue(manipNum);

											logOut << "[BIT_XOR]";
											std::cout << "[BIT_XOR]";
											break;
										}
										case modActionTypes::actTy_BIT_SHIFT_L:
										{
											actionOccured = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = neoParamVals.getParamValueNum();
											manipNum = manipNum << incomingValNum;
											neoParamVals.updateParamValue(manipNum);

											logOut << "[BIT_SHIFT_L]";
											std::cout << "[BIT_SHIFT_L]";
											break;
										}
										case modActionTypes::actTy_BIT_SHIFT_R:
										{
											actionOccured = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = neoParamVals.getParamValueNum();
											manipNum = manipNum >> incomingValNum;
											neoParamVals.updateParamValue(manipNum);

											logOut << "[BIT_SHIFT_R]";
											std::cout << "[BIT_SHIFT_R]";
											break;
										}
										case modActionTypes::actTy_BIT_ROTATE_L:
										{
											actionOccured = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = neoParamVals.getParamValueNum();
											unsigned int temp = manipNum;
											manipNum = manipNum << incomingValNum;
											temp = temp >> (32 - incomingValNum);
											manipNum |= temp;
											neoParamVals.updateParamValue(manipNum);

											logOut << "[BIT_ROTATE_L]";
											std::cout << "[BIT_ROTATE_L]";
											break;
										}
										case modActionTypes::actTy_BIT_ROTATE_R:
										{
											actionOccured = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = neoParamVals.getParamValueNum();
											unsigned int temp = manipNum;
											manipNum = manipNum >> incomingValNum;
											temp = temp << (32 - incomingValNum);
											manipNum |= temp;
											neoParamVals.updateParamValue(manipNum);

											logOut << "[BIT_ROTATE_R]";
											std::cout << "[BIT_ROTATE_R]";
											break;
										}
										case modActionTypes::actTy_RETARGET_PARAM:
										{
											if (hexStringToNum(currAction->value) >= 0)
											{
												actionOccured = 1;
												neoParamVals.saveParamToContents();
												neoParamVals = lava::paramTarget(*this, paramOffsetNum, lava::hexStringToNum(currAction->value));
												logOut << "[TARGET_PARAM]\n";
												std::cout << "[TARGET_PARAM]\n";
												logOut << "\t\t\tParameter Redirect Triggered: New target is Param Index " << hexStringToNum(currAction->value) << "\n";
												logOut << "\t\t\tParam Val: " << neoParamVals.getParamValueString();
												std::cout << "\t\t\tParameter Redirect Triggered: New target is Param Index " << hexStringToNum(currAction->value) << "\n";
												std::cout << "\t\t\tParam Val: " << neoParamVals.getParamValueString();
												if (neoParamVals.getParamTypeNum() == lava::movesetParamTypes::varTy_SCLR)
												{
													logOut << " (Scalar = " << neoParamVals.getParamValueNum() / lava::floatDenominator << ")";
													std::cout << " (Scalar = " << neoParamVals.getParamValueNum() / lava::floatDenominator << ")";
												}
												logOut << " (Redirected)\n";
												std::cout << " (Redirected)\n";
											}
											break;
										}
										case modActionTypes::actTy_CONVERT_PARAM:
										{
											logOut << "[TARGET_PARAM]";
											std::cout << "[TARGET_PARAM]";
											unsigned int incomingValNum = lava::hexStringToNum(currAction->value);
											if (incomingValNum < lava::movesetParamTypes::variableTypeCount)
											{
												switch (incomingValNum)
												{
													case lava::movesetParamTypes::varTy_INT:
													{
														if (neoParamVals.getParamTypeNum() == lava::movesetParamTypes::varTy_SCLR)
														{
															unsigned int manipNum = neoParamVals.getParamValueNum();
															manipNum /= lava::floatDenominator;
															neoParamVals.updateParamValue(manipNum);
														}
														break;
													}
													case lava::movesetParamTypes::varTy_SCLR:
													{
														doScalarActionPrint = 1;
														if (neoParamVals.getParamTypeNum() == lava::movesetParamTypes::varTy_INT)
														{
															unsigned int manipNum = neoParamVals.getParamValueNum();
															manipNum *= lava::floatDenominator;
															neoParamVals.updateParamValue(manipNum);
														}
														break;
													}
													case lava::movesetParamTypes::varTy_PNTR:
													{
														break;
													}
													case lava::movesetParamTypes::varTy_BOOL:
													{
														break;
													}
													case lava::movesetParamTypes::varTy_4:
													{
														break;
													}
													case lava::movesetParamTypes::varTy_VAR:
													{
														break;
													}
													case lava::movesetParamTypes::varTy_REQ:
													{
														break;
													}
													default:
													{
														break;
													}
												}
												neoParamVals.updateParamType(incomingValNum);
											}
											else
											{
												logOut << "\t\t\tTarget parameter type (" << incomingValNum << ") was invalid.\n";
												std::cout << "\t\t\tTarget parameter type (" << incomingValNum << ") was invalid.\n";
											}
										}
										default:
										{
											logOut << "[INVALID_TYPE]";
											std::cout << "[INVALID_TYPE]";
											break;
										}
									}

									if (actionOccured)
									{
										if (neoParamVals.getParamTypeNum() == lava::movesetParamTypes::varTy_SCLR || doScalarActionPrint)
										{
											logOut << " Value = " << currAction->value;
											std::cout << " Value = " << currAction->value;
											float incomingFlt = hexStringToNum(currAction->value) / lava::floatDenominator;
											logOut << " (Scalar = " << incomingFlt << ")";
											std::cout << " (Scalar = " << incomingFlt << ")";
											float intermediateFlt = lava::hexStringToNum(intermediateParamValString) / lava::floatDenominator;
											logOut << "\n\t\t\t\t" << intermediateParamValString << " (" << intermediateFlt << ") -> " << neoParamVals.getParamValueString() << " (" << neoParamVals.getParamValueNum() / lava::floatDenominator << ")\n";
											std::cout << "\n\t\t\t\t" << intermediateParamValString << " (" << intermediateFlt << ") -> " << neoParamVals.getParamValueString() << " (" << neoParamVals.getParamValueNum() / lava::floatDenominator << ")\n";
										}
										else
										{
											logOut << " Value = " << currAction->value << "\n\t\t\t\t" << intermediateParamValString << " -> " << neoParamVals.getParamValueString() << "\n";
											std::cout << " Value = " << currAction->value << "\n\t\t\t\t" << intermediateParamValString << " -> " << neoParamVals.getParamValueString() << "\n";
										}
									}
									else
									{
										std::cout << "\n";
										logOut << "\n";
									}
								}

								// Handle lock
								bool lockUsed = 0;
								for (std::size_t i = 0; i < 8; i++)
								{
									std::string lockApplicationStr = neoParamVals.getParamValueString();
									if (currMod->locked[i] == '1' && lockApplicationStr[i] != canonParamValString[i])
									{
										lockApplicationStr[i] = canonParamValString[i];
										lockUsed = 1;
									}
									neoParamVals.updateParamValue(lockApplicationStr);
								}
								if (lockUsed)
								{
									std::cout << "\t\t\tLock (" << currMod->locked << "): " << neoParamVals.getParamValueString() << "\n";
									logOut << "\t\t\tLock (" << currMod->locked << "): " << neoParamVals.getParamValueString() << "\n";
								}

								// Write result to contents.
								neoParamVals.saveParamToContents();

								// Report results:
								logOut << "\t\t\tFinal Value: " << contents.getBytes(4, neoParamVals.getParamOffsetNum() + neoParamVals.getParamIndexOffset() + 4, numGotten);
								std::cout << "\t\t\tFinal Value: " << contents.getBytes(4, neoParamVals.getParamOffsetNum() + neoParamVals.getParamIndexOffset() + 4, numGotten);
								// Do scalar report if necessary
								if (neoParamVals.getParamTypeNum() == lava::movesetParamTypes::varTy_SCLR || doScalarFinalPrint)
								{
									logOut << " (Scalar = " << neoParamVals.getParamValueNum() / lava::floatDenominator << ")";
									std::cout << " (Scalar = " << neoParamVals.getParamValueNum() / lava::floatDenominator << ")";
								}
								logOut << "\n";
								std::cout << "\n";
							}
							else
							{
								previousModUsed = 0;
							}
						}
						if (!matchFound)
						{
							logOut << "\n";
							std::cout << "\n";
						}
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

	paramTarget::paramTarget() {};
	paramTarget::paramTarget(movesetFile& parent, std::size_t paramOffsetIn, int paramIndexIn)
	{
		if (parent.contents.size)
		{
			parentPtr = &parent;
			targetParamIndex = paramIndexIn;
			targetParamIndexOffset = (8 * paramIndexIn);
			updateParamOffset(paramOffsetIn);
			std::size_t numGotten;
			updateParamType(parentPtr->contents.getBytes(4, this->paramOffsetNum + targetParamIndexOffset, numGotten));
			updateParamValue(parentPtr->contents.getBytes(4, this->paramOffsetNum + targetParamIndexOffset + 4, numGotten));

		}
	}
	int paramTarget::getParamIndex()
	{
		return targetParamIndex;
	}
	std::size_t paramTarget::getParamIndexOffset()
	{
		return targetParamIndexOffset;
	}
	std::size_t paramTarget::getParamOffsetNum()
	{
		return paramOffsetNum;
	}
	std::vector<char> paramTarget::getParamOffset()
	{
		return paramOffset;
	}
	std::size_t paramTarget::getParamTypeNum()
	{
		return paramTypeIdentifierNum;
	}
	std::vector<char> paramTarget::getParamType()
	{
		return paramTypeIdentifier;
	}
	std::size_t paramTarget::getParamValueNum()
	{
		return paramValueNum;
	}
	std::string paramTarget::getParamValueString()
	{
		return paramValueString;
	}
	std::vector<char> paramTarget::getParamValue()
	{
		return paramValue;
	}
	void paramTarget::updateParamOffset(std::size_t paramOffsetNumIn)
	{
		paramOffsetNum = paramOffsetNumIn;
		paramOffset = lava::numToHexVec(paramOffsetNum);
	}
	void paramTarget::updateParamOffset(std::vector<char> paramOffsetIn)
	{
		paramOffset = paramOffsetIn;
		paramOffsetNum = lava::hexVecToNum(paramOffset);
	}
	void paramTarget::updateParamType(std::size_t paramTypeIdentifierNumIn)
	{
		paramTypeIdentifierNum = paramTypeIdentifierNumIn;
		paramTypeIdentifier = lava::numToHexVec(paramTypeIdentifierNum);
	}
	void paramTarget::updateParamType(std::vector<char> paramTypeIdentifierIn)
	{
		paramTypeIdentifier = paramTypeIdentifier;
		paramTypeIdentifierNum = lava::hexVecToNum(paramTypeIdentifier);
	}
	void paramTarget::updateParamValue(std::size_t paramValueNumIn)
	{
		paramValueNum = paramValueNumIn;
		paramValueString = lava::numToHexStringWithPadding(paramValueNum, 8);
		paramValue = lava::numToHexVec(paramValueNum);
	}
	void paramTarget::updateParamValue(std::string paramValueStringIn)
	{
		paramValueString = paramValueStringIn;
		paramValueNum = lava::hexStringToNum(paramValueString);
		paramValue = lava::numToHexVec(paramValueNum);
	}
	void paramTarget::updateParamValue(std::vector<char> paramValueIn)
	{
		paramValue = paramValueIn;
		paramValueNum = lava::hexVecToNum(paramValue);
		paramValueString = lava::numToHexStringWithPadding(paramValueNum, 8);
	}

	bool paramTarget::saveParamToContents()
	{
		bool result = 1;
		result &= parentPtr->contents.setBytes(paramTypeIdentifier, paramOffsetNum + targetParamIndexOffset);
		result &= parentPtr->contents.setBytes(paramValue, paramOffsetNum + targetParamIndexOffset + 4);
		return result;
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
