#include "lavaMontyPythonReimp.h"

namespace lava
{
	std::ofstream changelogStream = std::ofstream();

	bool isHexChar(char charIn)
	{
		return (validHexChars.find(charIn) != std::string::npos);
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
		for (std::size_t i = 0; i < 8; i++)
		{
			if (!lava::isHexChar(result[i]) && !(result[i] == 'X' && XAllowed))
			{
				result[i] = '0';
			}
		}
		return result;
	}
	int parseXMLValueAsNum(const std::string& stringIn, bool allowNeg, int defaultVal)
	{
		int result = defaultVal;
		std::string manipStr = stringIn;
		int base = (manipStr.find("0x") == 0) ? 16 : 10;
		char* res = nullptr;
		result = std::strtoul(manipStr.c_str(), &res, base);
		if (res != (manipStr.c_str() + manipStr.size()))
		{
			result = defaultVal;
		}
		if (result < 0 && !allowNeg)
		{
			result = defaultVal;
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
	float hexRepToFloat(unsigned int hexIn)
	{
		lava::INT_FLT_BUNDLE temp;
		temp.hex = hexIn;
		return temp.flt;
	}
	unsigned int floatToHexRep(float floatIn)
	{
		lava::INT_FLT_BUNDLE temp;
		temp.flt = floatIn;
		return temp.hex;
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
				currPatch.sourceFile = fileIn;
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
						std::size_t targetNumber = 0;
						for (pugi::xml_node patchTarget = patchChild.first_child(); patchTarget; patchTarget = patchTarget.next_sibling())
						{
							movesetPatchTarget currTarget;
							currTarget.name = "Target #" + std::to_string(targetNumber);
							for (pugi::xml_attribute targetAttr = patchTarget.first_attribute(); targetAttr; targetAttr = targetAttr.next_attribute())
							{
								if (targetAttr.name() == "name")
								{
									currTarget.name = targetAttr.as_string();
								}
								else if (targetAttr.name() == "signature")
								{
									currTarget.signature = lava::numToHexVec(lava::parseXMLValueAsNum(targetAttr.as_string(), 1));
								}
								else if (targetAttr.name() == "paramIndex")
								{
									currTarget.paramIndex = lava::parseXMLValueAsNum(targetAttr.as_string(), 0, INT_MAX);
								}
								else if (targetAttr.name() == "paramType")
								{
									currTarget.paramType = lava::parseXMLValueAsNum(targetAttr.as_string(), 0, INT_MAX);
								}
							}
							if (currTarget.signature != lava::numToHexVec(INT_MAX))
							{
								if (currTarget.paramIndex != INT_MAX)
								{
									currPatch.targets.push_back(currTarget);
								}
								else
								{
									std::cerr << "[ERROR] Error in parsing param index for Target \"" << currTarget.name << "\" in moveset patch XML (" << fileIn << ").\n Ensure that this value is a positive integer value.\n";
								}
							}
							else
							{
								std::cerr << "[ERROR] Error in parsing signature for Target \"" << currTarget.name << "\" in moveset patch XML (" << fileIn << ").\n Ensure that this is a valid hexadecimal number.\n";
							}
							targetNumber++;
						}
					}
					else if (patchChild.name() == "modifications")
					{
						std::size_t modNumber = 0;
						for (pugi::xml_node patchModification = patchChild.first_child(); patchModification; patchModification = patchModification.next_sibling())
						{
							movesetPatchMod currMod;
							currMod.name = "Modification #" + std::to_string(modNumber);
							for (pugi::xml_attribute patchModAttr = patchModification.first_attribute(); patchModAttr; patchModAttr = patchModAttr.next_attribute())
							{
								if (patchModAttr.name() == "name")
								{
									currMod.name = patchModAttr.as_string();
								}
								else if (patchModAttr.name() == "match")
								{
									currMod.match = lava::sanitizeHexStrInput(patchModAttr.as_string(), 1);
								}
								else if (patchModAttr.name() == "extraCondition")
								{
									currMod.extraCondition = lava::parseXMLValueAsNum(patchModAttr.as_string(), 0, INT_MAX);
								}
								else if (patchModAttr.name() == "evalMethod")
								{
									currMod.evalMethod = lava::parseXMLValueAsNum(patchModAttr.as_string(), 0, INT_MAX);
								}
								else if (patchModAttr.name() == "lock")
								{
									currMod.locked = lava::sanitizeHexStrInput(patchModAttr.as_string(), 0);
								}
								else if (patchModAttr.name() == "paramIndexRedirect")
								{
									currMod.paramIndexRedirect = lava::parseXMLValueAsNum(patchModAttr.as_string(), 0, INT_MAX);
								}
							}
							std::size_t actionNumber = 0;
							for (pugi::xml_node modAction = patchModification.first_child(); modAction; modAction = modAction.next_sibling())
							{
								movesetPatchModAction currAction;
								currAction.name = "Action #" + std::to_string(actionNumber);
								for (pugi::xml_attribute actionAttr = modAction.first_attribute(); actionAttr; actionAttr = actionAttr.next_attribute())
								{
									if (actionAttr.name() == "name")
									{
										currAction.name = actionAttr.as_string();
									}
									else if (actionAttr.name() == "type")
									{
										currAction.actionType = lava::parseXMLValueAsNum(actionAttr.as_string(), 0, INT_MAX);
									}
									else if (actionAttr.name() == "value")
									{
										currAction.value = lava::sanitizeHexStrInput(actionAttr.as_string(), 0);
									}
								}
								currMod.actions.push_back(currAction);
								actionNumber++;
							}
							modNumber++;
							currPatch.modifications.push_back(currMod);
						}
					}
					else if (patchChild.name() == "directApply")
					{
						std::string baseName = "DirectApply";
						unsigned int templateAction = lava::modActionTypes::actTy_REPLACE;
						int templateEvalMethod = matchEvaluationMethodTypes::mtEvl_EQUALS;
						int templateRedirectParam = INT_MAX;
						std::string templateLock = "00000000";
						for (pugi::xml_attribute directApplyAttr = patchChild.first_attribute(); directApplyAttr; directApplyAttr = directApplyAttr.next_attribute())
						{
							if (directApplyAttr.name() == "name")
							{
								baseName = directApplyAttr.as_string();
							}
							else if (directApplyAttr.name() == "type")
							{
								templateAction = lava::parseXMLValueAsNum(directApplyAttr.as_string(), 0, INT_MAX);
							}
							else if (directApplyAttr.name() == "evalMethod")
							{
								templateEvalMethod = lava::parseXMLValueAsNum(directApplyAttr.as_string(), 0, INT_MAX);
							}
							else if (directApplyAttr.name() == "paramIndexRedirect")
							{
								templateRedirectParam = lava::parseXMLValueAsNum(directApplyAttr.as_string(), 0, INT_MAX);
							}
							else if (directApplyAttr.name() == "lock")
							{
								templateLock = lava::sanitizeHexStrInput(directApplyAttr.as_string(), 0);
							}
						}
						if (templateAction != INT_MAX)
						{
							for (pugi::xml_node directApplyChild = patchChild.first_child(); directApplyChild; directApplyChild = directApplyChild.next_sibling())
							{
								if (directApplyChild.type() == pugi::xml_node_type::node_pcdata)
								{
									std::string commentChars = "#";
									std::stringstream entryStream;
									entryStream.str(directApplyChild.value());
									std::string manipStr = "";
									std::size_t entryNum = 0;
									while (std::getline(entryStream, manipStr))
									{
										// Erase all instances of tabs and spaces from the string
										manipStr.erase(std::remove(manipStr.begin(), manipStr.end(), '\t'), manipStr.end());
										manipStr.erase(std::remove(manipStr.begin(), manipStr.end(), ' '), manipStr.end());
										if (manipStr.size())
										{
											std::size_t delimLoc = manipStr.find(',');
											if (commentChars.find(manipStr[0]) == std::string::npos)
											{
												if (delimLoc != std::string::npos && delimLoc > 0 && delimLoc < (manipStr.size() - 1))
												{
													std::string match = manipStr.substr(0, delimLoc);
													match = lava::sanitizeHexStrInput(match, 1);
													std::string value = manipStr.substr(delimLoc + 1, std::string::npos);
													value = lava::sanitizeHexStrInput(value, 0);

													lava::movesetPatchMod tempMod;
													tempMod.name = baseName + "[" + std::to_string(entryNum) + "]";
													tempMod.locked = templateLock;
													tempMod.evalMethod = templateEvalMethod;
													tempMod.paramIndexRedirect = templateRedirectParam;
													tempMod.match = lava::sanitizeHexStrInput(match, 1);
													lava::movesetPatchModAction tempAction;
													tempAction.name = "Action #0";
													tempAction.actionType = templateAction;
													tempAction.value = lava::sanitizeHexStrInput(value, 0);
													tempMod.actions.push_back(tempAction);
													currPatch.modifications.push_back(tempMod);
												}
											}
											entryNum++;
										}
									}
								}
							}
						}
					}
				}
				result.push_back(currPatch);
			}
		}
		else
		{
			std::cerr << "[ERROR] Failed to parse moveset patch XML file: " << parseRes.description() << "\n\n";
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
		std::cout << "Loaded " << size << " byte(s) of data.\n\n";
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

	bool movesetFile::init(std::string filePathIn, std::ostream& logStream)
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
				movesetLength = lava::hexVecToNum(contents.getBytes(4, lava::PACFileHeaderLength, numGotten));
				dataLength = lava::hexVecToNum(contents.getBytes(4, lava::PACFileHeaderLength + 0x4, numGotten));
				dataOffset = lava::PACFileHeaderLength + lava::movesetHeaderLength;
				std::cout << "\tMoveset Section[0x80]: Total Length = 0x" << movesetLength << ", Data Length = 0x" << dataLength << "\n";
				offsetSectionCount = lava::hexVecToNum(contents.getBytes(4, lava::PACFileHeaderLength + 0x8, numGotten));
				offsetSectionOffset = dataOffset + dataLength;
				std::cout << "\tOffset Section[0x" << offsetSectionOffset << "]: Entry Count = 0x" << offsetSectionCount << "\n";
				dataTableCount = lava::hexVecToNum(contents.getBytes(4, lava::PACFileHeaderLength + 0xC, numGotten));
				dataTableOffset = offsetSectionOffset + (offsetSectionCount * 0x4);
				std::cout << "\tData Table Section[0x" << dataTableOffset << "]: Entry Count = 0x" << dataTableCount << "\n";
				externalDataCount = lava::hexVecToNum(contents.getBytes(4, lava::PACFileHeaderLength + 0x10, numGotten));
				externalDataOffset = dataTableOffset + (dataTableCount * 0x8);
				std::cout << "\tExternal Data Table Section[0x" << externalDataOffset << "]: Entry Count = 0x" << externalDataCount << "\n";
				tablesEnd = externalDataOffset + (externalDataCount * 0x8);
				std::cout << "\tTables End[0x" << tablesEnd << "]";
				std::cout << "\n" << std::dec;
				/*std::cout << "\tData Table Contents:\n";
				summarizeTable(dataTableOffset, dataTableCount, lava::PACFileHeaderLength + lava::movesetHeaderLength, "\t\t");
				std::cout << "\tExternal Data Table Contents:\n";
				summarizeTable(externalDataOffset, externalDataCount, 0x0, "\t\t");*/
				std::cout << "\n";
				if (logStream.good())
				{
					logStream << "Header Info:\n" << std::hex;
					logStream << "\tMoveset Section[0x80]: Total Length = 0x" << movesetLength << ", Data Length = 0x" << dataLength << "\n";
					logStream << "\tOffset Section[0x" << offsetSectionOffset << "]: Entry Count = 0x" << offsetSectionCount << "\n";
					logStream << "\tData Table Section[0x" << dataTableOffset << "]: Entry Count = 0x" << dataTableCount << "\n";
					logStream << "\tExternal Data Table Section[0x" << externalDataOffset << "]: Entry Count = 0x" << externalDataCount << "\n";
					logStream << "\tTables End[0x" << tablesEnd << "]";
					logStream << "\n\n" << std::dec;
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
	void movesetFile::summarizeAttributeSection(std::ostream& output)
	{
		std::size_t numGotten = SIZE_MAX;
		std::vector<char> currAttr = { 0, 0, 0, 0 };
		lava::INT_FLT_BUNDLE currAttrVal;
		for (std::size_t attributeItr = dataOffset; attributeItr < (dataOffset + lava::canonAttributeSectionLength); attributeItr += lava::canonAttributeLengthInBytes)
		{
			currAttr = contents.getBytes(lava::canonAttributeLengthInBytes, attributeItr, numGotten);
			currAttrVal.hex = lava::hexVecToNum(currAttr);
			output << "Attributes[0x" << lava::numToHexStringWithPadding(attributeItr - dataOffset, 4) << "]: ";
			if (currAttrVal.hex > 0x10000000)
			{
				output << currAttrVal.flt;
			}
			else
			{
				output << currAttrVal.hex;
			}
			output << "\n";
		}
	}

	lava::movesetPatchResult movesetFile::applyMovesetPatch(const lava::movesetPatch& patchIn, std::ostream& logStream)
	{
		// Initialize results struct
		lava::movesetPatchResult results;
		// Store sizes of respective lists
		std::size_t targetCount = patchIn.targets.size();
		std::size_t modificationCount = patchIn.modifications.size();
		// Resize result list
		results.timesTargetsHit.resize(targetCount, std::vector<std::size_t>(modificationCount, 0));
		
		// Format float precision for output streams
		std::cout << std::fixed;
		std::cout << std::setprecision(3);
		logStream << std::fixed;
		logStream << std::setprecision(3);

		// Initialize variables for reuse in the loop
		std::size_t numGotten = 0;
		std::vector<std::size_t> searchPings = {};

		// Initialize printType enum. This is local to this function because it's only used here.
		// If that ever changes, it'll go with the other enums probably.
		enum class printType
		{
			integerPrint = 0,
			scalarPrint,
			floatPrint
		};

		// Initialize Attribute Hack Vars
		bool doingAttributeHack = 0;

		// Loop through every target
		for (std::size_t targetItr = 0; targetItr < targetCount; targetItr++)
		{
			// Grab pointer to current target, and ping for its signature within the data section
			const movesetPatchTarget* currTarget = &patchIn.targets[targetItr];
			std::cout << "\"" << currTarget->name << "\" [0x" << lava::hexVecToHexStringWithPadding(currTarget->signature, lava::canonParamLengthStr) << "] (Param " << currTarget->paramIndex;
			logStream << "\"" << currTarget->name << "\" [0x" << lava::hexVecToHexStringWithPadding(currTarget->signature, lava::canonParamLengthStr) << "] (Param " << currTarget->paramIndex;
			// If a paramType was specified, report it
			if (currTarget->paramType != INT_MAX)
			{
				std::cout << ", of Type " << currTarget->paramType;
				logStream << ", of Type " << currTarget->paramType;
			}
			std::cout << "):\n";
			logStream << "):\n";
			
			if (lava::hexVecToNum(currTarget->signature) == attributeHackActivationSignature)
			{
				searchPings = {dataOffset};
				doingAttributeHack = 1;
			}
			else
			{
				searchPings = contents.searchMultiple(currTarget->signature, dataOffset, dataOffset + dataLength);
				doingAttributeHack = 0;
			}

			// Loop through every received ping
			for (std::size_t pingItr = 0; pingItr < searchPings.size(); pingItr++)
			{
				// Receive and validate that ping's parameter offset
				std::size_t currPing = searchPings[pingItr];
				std::vector<char> paramOffset;
				if (doingAttributeHack)
				{
					paramOffset = lava::numToHexVec(0x00);
					numGotten = 4;
				}
				else
				{
					paramOffset = contents.getBytes(4, currPing + currTarget->signature.size(), numGotten);
				}
				if (numGotten == 4)
				{
					// Convert that offset to a size_t, and apply dataOffset (makes offset relative to beginning of .PAC file instead of beginning of moveset section
					std::size_t paramOffsetNum = lava::hexVecToNum(paramOffset) + dataOffset;

					// Determine whether or not that's actually a valid location or not, and if so, proceed.
					if (paramOffsetNum < (dataOffset + dataLength))
					{
						// Create our param target. This holds the current value of the target parameter, as well as where it's stored in the file, and is the variable that we'll be modifying to make changes to the parameter's value and type.
						lava::paramTarget currParamVals(*this, paramOffsetNum, currTarget->paramIndex, doingAttributeHack);

						// Process target paramType matching. If no type was specified, or if one was specified and it matches our found type, proceed.
						if (currTarget->paramType == INT_MAX || currParamVals.getParamTypeNum() == currTarget->paramType)
						{
							// Report location of current ping, as well as its paramOffset. Note, the first paramOffset we report is non .pac adjusted since that's the one you'll see in PSAC, which is what I expect most people will be looking for. The adjusted value is provided second.
							logStream << "\t[0x" << numToHexStringWithPadding(currPing - dataOffset, 8) << "]: Params @ 0x" << numToHexStringWithPadding(paramOffsetNum - dataOffset, 8);
							std::cout << "\t[0x" << numToHexStringWithPadding(currPing - dataOffset, 8) << "]: Params @ 0x" << numToHexStringWithPadding(paramOffsetNum - dataOffset, 8);

							// Initialize values for loop
							bool redirectUsed = 0;
							bool previousModUsed = 0;
							std::size_t modItr = 0;
							bool matchFound = 0;

							printType actionPrintType = printType::integerPrint;
							printType finalPrintType = printType::integerPrint;
							
							const movesetPatchMod* currMod = nullptr;

							// Do initial param val reporting
							logStream << ", Param Val: " << currParamVals.getParamValueString();
							std::cout << ", Param Val: " << currParamVals.getParamValueString();
							if (currParamVals.getParamTypeNum() == lava::movesetParamTypes::varTy_SCLR)
							{
								logStream << " (Scalar = " << currParamVals.getParamValueNum() / lava::floatDenominator << ")";
								std::cout << " (Scalar = " << currParamVals.getParamValueNum() / lava::floatDenominator << ")";
							}
							if (currParamVals.getParamTypeNum() == lava::movesetParamTypes::varTy_ATTRIBUTE_FLT)
							{
								lava::INT_FLT_BUNDLE currParamIFB;
								currParamIFB.hex = currParamVals.getParamValueNum();
								logStream << " (Float = " << currParamIFB.flt << ")";
								std::cout << " (Float = " << currParamIFB.flt << ")";
							}
							logStream << "\n";
							std::cout << "\n";

							// Iterate through every modification
							for (std::size_t modItr = 0; modItr < modificationCount; modItr++)
							{
								// Record pointer to current modification
								currMod = &patchIn.modifications[modItr];

								// Handle param redirects
								// If a redirect index was specified...
								if (currMod->paramIndexRedirect != INT_MAX)
								{
									//... reinitialize the param bundle to point to the new param.
									currParamVals = lava::paramTarget(*this, paramOffsetNum, currMod->paramIndexRedirect, doingAttributeHack);
									// Record that the redirect happened (used for reporting later).
									redirectUsed = 1;
								}
								// Otherwise...
								else
								{
									//... ensure we're on the parameter originally specified by the target.
									currParamVals = lava::paramTarget(*this, paramOffsetNum, currTarget->paramIndex, doingAttributeHack);
									// Record that no redirect happened.
									redirectUsed = 0;
								}

								// Set final print type based on param type.
								switch (currParamVals.getParamTypeNum())
								{
									case lava::movesetParamTypes::varTy_SCLR:
									{
										finalPrintType = printType::scalarPrint;
										break;
									}
									case lava::movesetParamTypes::varTy_ATTRIBUTE_FLT:
									{
										finalPrintType = printType::floatPrint;
										break;
									}
									default:
									{
										finalPrintType = printType::integerPrint;
										break;
									}
								}

								// Initiate match evaluation result variable
								bool matchEvalRes = 0;
								// Build the version of the match string we'll be using. This just replaces any passthrough chars ('X's) with the digit in the corresponding slot of the target param, ensuring they're equal.
								std::string tempMatchEvalStr = currMod->match;
								for (std::size_t i = 0; i < tempMatchEvalStr.size(); i++)
								{
									if (tempMatchEvalStr[i] == 'X')
									{
										tempMatchEvalStr[i] = currParamVals.getParamValueString()[i];
									}
								}
								// Actually evaluate the various methods.
								switch (currMod->evalMethod)
								{
									case lava::matchEvaluationMethodTypes::mtEvl_EQUALS:
									{
										matchEvalRes = hexStrComp(tempMatchEvalStr, currParamVals.getParamValueString());
										break;
									}
									case lava::matchEvaluationMethodTypes::mtEvl_NOT_EQUALS:
									{
										matchEvalRes = !hexStrComp(tempMatchEvalStr, currParamVals.getParamValueString());
										break;
									}
									case lava::matchEvaluationMethodTypes::mtEvl_GREATER:
									{
										if (currParamVals.getParamTypeNum() != lava::movesetParamTypes::varTy_ATTRIBUTE_FLT)
										{
											matchEvalRes = lava::hexStringToNum(tempMatchEvalStr) < currParamVals.getParamValueNum();
										}
										else
										{
											lava::INT_FLT_BUNDLE paramValFlt;
											paramValFlt.hex = currParamVals.getParamValueNum();
											lava::INT_FLT_BUNDLE matchFlt;
											matchFlt.hex = lava::hexStringToNum(tempMatchEvalStr);
											matchEvalRes = matchFlt.flt < paramValFlt.flt;
										}
										break;
									}
									case lava::matchEvaluationMethodTypes::mtEvl_GREATER_OE:
									{
										if (currParamVals.getParamTypeNum() != lava::movesetParamTypes::varTy_ATTRIBUTE_FLT)
										{
											matchEvalRes = lava::hexStringToNum(tempMatchEvalStr) <= currParamVals.getParamValueNum();
										}
										else
										{
											lava::INT_FLT_BUNDLE paramValFlt;
											paramValFlt.hex = currParamVals.getParamValueNum();
											lava::INT_FLT_BUNDLE matchFlt;
											matchFlt.hex = lava::hexStringToNum(tempMatchEvalStr);
											matchEvalRes = matchFlt.flt <= paramValFlt.flt;
										}
										break;
									}
									case lava::matchEvaluationMethodTypes::mtEvl_LESSER:
									{
										if (currParamVals.getParamTypeNum() != lava::movesetParamTypes::varTy_ATTRIBUTE_FLT)
										{
											matchEvalRes = lava::hexStringToNum(tempMatchEvalStr) > currParamVals.getParamValueNum();
										}
										else
										{
											lava::INT_FLT_BUNDLE paramValFlt;
											paramValFlt.hex = currParamVals.getParamValueNum();
											lava::INT_FLT_BUNDLE matchFlt;
											matchFlt.hex = lava::hexStringToNum(tempMatchEvalStr);
											matchEvalRes =  matchFlt.flt > paramValFlt.flt;
										}
										break;
									}
									case lava::matchEvaluationMethodTypes::mtEvl_LESSER_OE:
									{
										if (currParamVals.getParamTypeNum() != lava::movesetParamTypes::varTy_ATTRIBUTE_FLT)
										{
											matchEvalRes = lava::hexStringToNum(tempMatchEvalStr) >= currParamVals.getParamValueNum();
										}
										else
										{
											lava::INT_FLT_BUNDLE paramValFlt;
											paramValFlt.hex = currParamVals.getParamValueNum();
											lava::INT_FLT_BUNDLE matchFlt;
											matchFlt.hex = lava::hexStringToNum(tempMatchEvalStr);
											matchEvalRes = matchFlt.flt >= paramValFlt.flt;
										}
										break;
									}
									case lava::matchEvaluationMethodTypes::mtEvl_BIT_AND:
									{
										matchEvalRes = lava::hexStringToNum(tempMatchEvalStr) & currParamVals.getParamValueNum();
										break;
									}
									default:
									{
										matchEvalRes = 0;
										break;
									}
								}

								// Evaluate status of specified condition.
								switch (currMod->extraCondition)
								{
									case lava::extraConditionTypes::exCon_AND_PREV_USED:
									{
										matchEvalRes &= previousModUsed;
										break;
									}
									case lava::extraConditionTypes::exCon_AND_PREV_NOT_USED:
									{
										matchEvalRes &= !previousModUsed;
										break;
									}
									case lava::extraConditionTypes::exCon_OR_PREV_USED:
									{
										matchEvalRes |= previousModUsed;
										break;
									}
									case lava::extraConditionTypes::exCon_OR_PREV_NOT_USED:
									{
										matchEvalRes |= !previousModUsed;
										break;
									}
									case lava::extraConditionTypes::exCon_XOR_PREV_USED:
									{
										matchEvalRes ^= previousModUsed;
										break;
									}
									case lava::extraConditionTypes::exCon_XOR_PREV_NOT_USED:
									{
										matchEvalRes ^= !previousModUsed;
										break;
									}
									default:
									{
										break;
									}
								}

								// If after processing the match evaluation method (and the extra condition if it one was given), continue with applying any actions.
								if (matchEvalRes)
								{
									// Record that the modification was used and report it to the console.
									matchFound = 1;
									previousModUsed = 1;
									results.timesTargetsHit[targetItr][modItr]++;

									// Do logging
									logStream << "\t\t\"" << currMod->name << "\":\n";
									std::cout << "\t\t\"" << currMod->name << "\":\n";
									// Redirect info is only printed if relevant
									if (redirectUsed)
									{
										logStream << "\t\t(Redirect Triggered, New Target Index is " << currMod->paramIndexRedirect << ")\n";
										std::cout << "\t\t(Redirect Triggered, New Target Index is " << currMod->paramIndexRedirect << ")\n";
									}
									// Print info on the match method
									logStream << "\t\tFound match (Match:" << currMod->match << " ";
									std::cout << "\t\tFound match (Match:" << currMod->match << " ";
									switch (currMod->evalMethod)
									{
										case lava::matchEvaluationMethodTypes::mtEvl_EQUALS:
										{
											std::cout << "==";
											logStream << "==";
											break;
										}
										case lava::matchEvaluationMethodTypes::mtEvl_NOT_EQUALS:
										{
											std::cout << "!=";
											logStream << "!=";
											break;
										}
										case lava::matchEvaluationMethodTypes::mtEvl_GREATER:
										{
											std::cout << ">";
											logStream << ">";
											break;
										}
										case lava::matchEvaluationMethodTypes::mtEvl_GREATER_OE:
										{
											std::cout << ">=";
											logStream << ">=";
											break;
										}
										case lava::matchEvaluationMethodTypes::mtEvl_LESSER:
										{
											std::cout << "<";
											logStream << "<";
											break;
										}
										case lava::matchEvaluationMethodTypes::mtEvl_LESSER_OE:
										{
											std::cout << "<=";
											logStream << "<=";
											break;
										}
										case lava::matchEvaluationMethodTypes::mtEvl_BIT_AND:
										{
											std::cout << "&";
											logStream << "&";
											break;
										}
										default:
										{
											break;
										}
									}
									logStream << " Param Value:" << currParamVals.getParamValueString() << ")\n";
									std::cout << " Param Value:" << currParamVals.getParamValueString() << ")\n";
									// Extra condition info is also only printed if relevant
									if (currMod->extraCondition)
									{
										std::cout << "\t\tExtra Condition also met:";
										logStream << "\t\tExtra Condition also met:";
										switch (currMod->extraCondition)
										{
										case lava::extraConditionTypes::exCon_AND_PREV_USED:
										{
											std::cout << " [AND_PREV]\n";
											logStream << " [AND_PREV]\n";
											break;
										}
										case lava::extraConditionTypes::exCon_AND_PREV_NOT_USED:
										{
											std::cout << " [AND_NOT_PREV]\n";
											logStream << " [AND_NOT_PREV]\n";
											break;
										}
										case lava::extraConditionTypes::exCon_OR_PREV_USED:
										{
											std::cout << " [OR_PREV]\n";
											logStream << " [OR_PREV]\n";
											break;
										}
										case lava::extraConditionTypes::exCon_OR_PREV_NOT_USED:
										{
											std::cout << " [OR_NOT_PREV]\n";
											logStream << " [OR_NOT_PREV]\n";
											break;
										}
										case lava::extraConditionTypes::exCon_XOR_PREV_USED:
										{
											std::cout << " [XOR_PREV]\n";
											logStream << " [XOR_PREV]\n";
											break;
										}
										case lava::extraConditionTypes::exCon_XOR_PREV_NOT_USED:
										{
											std::cout << " [XOR_NOT_PREV]\n";
											logStream << " [XOR_NOT_PREV]\n";
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
									const std::string canonParamValString = currParamVals.getParamValueString();
									std::string intermediateParamValString = currParamVals.getParamValueString();
									bool doActionPrint = 0;

									// Iterate through each action:
									for (std::size_t actionItr = 0; actionItr < currMod->actions.size(); actionItr++)
									{
										// Store intermediate paramString
										intermediateParamValString = currParamVals.getParamValueString();

										// Reset action print management variables
										doActionPrint = 0;
										actionPrintType = finalPrintType;

										// Store a pointer to the current action
										currAction = &currMod->actions[actionItr];

										// Do changelog reporting
										logStream << "\t\t\t\"" << currAction->name << "\": ";
										std::cout << "\t\t\t\"" << currAction->name << "\": ";
										// Perform the appropriate changes based on the type of action
										switch (currAction->actionType)
										{
											case modActionTypes::actTy_DO_NOTHING:
											{
												logStream << "[NOP]";
												std::cout << "[NOP]";
												break;
											}
											case modActionTypes::actTy_REPLACE:
											{
												doActionPrint = 1;

												currParamVals.updateParamValue(currAction->value);

												logStream << "[REP]";
												std::cout << "[REP]";
												break;
											}
											case modActionTypes::actTy_RANDOM:
											{
												doActionPrint = 1;

												// Grab the incoming max value
												// We add 1 because the max value is inclusive
												unsigned int incomingNum = lava::hexStringToNum(currAction->value) + 1;
												// Ensure that our value isn't now equal to 0, as dividing by 0 is a bad idea
												incomingNum = (incomingNum == 0) ? UINT_MAX : incomingNum;
												// Generate a full 4 byte random number.
												// This is done in two rand calls because RAND_MAX is 2^16-1 on my platform, so it can only fill 2 bytes.
												// It can never be less than that, and it won't matter if it's ever more than that shifting it won't matter
												unsigned int randomNum = rand() | (rand() << 16);
												// Do the mod
												randomNum %= incomingNum;
												currParamVals.updateParamValue(randomNum);

												logStream << "[RAND]";
												std::cout << "[RAND]";
												break;
											}
											case modActionTypes::actTy_FLT_RANDOM:
											{
												doActionPrint = 1;
												actionPrintType = printType::floatPrint;

												// Get random number between 0 and 1
												float tempFloat = float(rand()) / float((RAND_MAX));
												lava::INT_FLT_BUNDLE incomingValNum;
												// Create float representation of the incoming maximum value
												incomingValNum.hex = hexStringToNum(currAction->value);
												// Multiply that float representation by the random number between 0 and 1
												incomingValNum.flt *= tempFloat;
												currParamVals.updateParamValue(incomingValNum.hex);

												logStream << "[FLT_RAND]";
												std::cout << "[FLT_RAND]";
												break;
											}
											case modActionTypes::actTy_INT_ADD:
											{
												doActionPrint = 1;

												unsigned int incomingValNum = hexStringToNum(currAction->value);
												unsigned int manipNum = currParamVals.getParamValueNum();
												manipNum += incomingValNum;
												currParamVals.updateParamValue(manipNum);

												logStream << "[INT_ADD]";
												std::cout << "[INT_ADD]";
												break;
											}
											case modActionTypes::actTy_INT_SUB:
											{
												doActionPrint = 1;

												unsigned int incomingValNum = hexStringToNum(currAction->value);
												unsigned int manipNum = currParamVals.getParamValueNum();
												manipNum -= incomingValNum;
												currParamVals.updateParamValue(manipNum);

												logStream << "[INT_SUB]";
												std::cout << "[INT_SUB]";
												break;
											}
											case modActionTypes::actTy_INT_MUL:
											{
												doActionPrint = 1;

												unsigned int incomingValNum = hexStringToNum(currAction->value);
												unsigned int manipNum = currParamVals.getParamValueNum();
												manipNum *= incomingValNum;
												currParamVals.updateParamValue(manipNum);

												logStream << "[INT_MUL]";
												std::cout << "[INT_MUL]";
												break;
											}
											case modActionTypes::actTy_INT_DIV:
											{
												doActionPrint = 1;

												unsigned int incomingValNum = hexStringToNum(currAction->value);
												unsigned int manipNum = currParamVals.getParamValueNum();
												manipNum /= incomingValNum;
												currParamVals.updateParamValue(manipNum);

												logStream << "[INT_DIV]";
												std::cout << "[INT_DIV]";
												break;
											}
											case modActionTypes::actTy_INT_MOD:
											{
												doActionPrint = 1;

												unsigned int incomingValNum = hexStringToNum(currAction->value);
												unsigned int manipNum = currParamVals.getParamValueNum();
												manipNum %= incomingValNum;
												currParamVals.updateParamValue(manipNum);

												logStream << "[INT_MOD]";
												std::cout << "[INT_MOD]";
												break;
											}
											case modActionTypes::actTy_SCLR_ADD:
											{
												doActionPrint = 1;
												actionPrintType = printType::scalarPrint;

												unsigned int incomingValNum = hexStringToNum(currAction->value);
												unsigned int manipNum = currParamVals.getParamValueNum();
												manipNum += incomingValNum;
												currParamVals.updateParamValue(manipNum);

												logStream << "[SCLR_ADD]";
												std::cout << "[SCLR_ADD]";
												break;
											}
											case modActionTypes::actTy_SCLR_SUB:
											{
												doActionPrint = 1;
												actionPrintType = printType::scalarPrint;

												unsigned int incomingValNum = hexStringToNum(currAction->value);
												unsigned int manipNum = currParamVals.getParamValueNum();
												manipNum -= incomingValNum;
												currParamVals.updateParamValue(manipNum);

												logStream << "[SCLR_SUB]";
												std::cout << "[SCLR_SUB]";
												break;
											}
											case modActionTypes::actTy_SCLR_MUL:
											{
												doActionPrint = 1;
												actionPrintType = printType::scalarPrint;

												float incomingValNum = hexStringToNum(currAction->value) / lava::floatDenominator;
												unsigned int manipNum = currParamVals.getParamValueNum();
												manipNum *= incomingValNum;
												currParamVals.updateParamValue(manipNum);

												logStream << "[SCLR_MUL]";
												std::cout << "[SCLR_MUL]";
												break;
											}
											case modActionTypes::actTy_SCLR_DIV:
											{
												doActionPrint = 1;
												actionPrintType = printType::scalarPrint;

												float incomingValNum = hexStringToNum(currAction->value) / lava::floatDenominator;
												unsigned int manipNum = currParamVals.getParamValueNum();
												manipNum /= incomingValNum;
												currParamVals.updateParamValue(manipNum);

												logStream << "[SCLR_DIV]";
												std::cout << "[SCLR_DIV]";
												break;
											}
											case modActionTypes::actTy_SCLR_MOD:
											{
												doActionPrint = 1;
												actionPrintType = printType::scalarPrint;

												unsigned int incomingValNum = hexStringToNum(currAction->value);
												unsigned int manipNum = currParamVals.getParamValueNum();
												manipNum %= incomingValNum;
												currParamVals.updateParamValue(manipNum);

												logStream << "[SCLR_MOD]";
												std::cout << "[SCLR_MOD]";
												break;
											}
											case modActionTypes::actTy_FLT_ADD:
											{
												doActionPrint = 1;
												actionPrintType = printType::floatPrint;

												lava::INT_FLT_BUNDLE incomingValNum = {UINT_MAX};
												incomingValNum.hex = hexStringToNum(currAction->value);
												lava::INT_FLT_BUNDLE manipNum = { UINT_MAX };
												manipNum.hex = currParamVals.getParamValueNum();
												manipNum.flt += incomingValNum.flt;
												currParamVals.updateParamValue(manipNum.hex);

												logStream << "[FLT_ADD]";
												std::cout << "[FLT_ADD]";
												break;
											}
											case modActionTypes::actTy_FLT_SUB:
											{
												doActionPrint = 1;
												actionPrintType = printType::floatPrint;

												lava::INT_FLT_BUNDLE incomingValNum;
												incomingValNum.hex = hexStringToNum(currAction->value);
												lava::INT_FLT_BUNDLE manipNum;
												manipNum.hex = currParamVals.getParamValueNum();
												manipNum.flt -= incomingValNum.flt;
												currParamVals.updateParamValue(manipNum.hex);

												logStream << "[FLT_SUB]";
												std::cout << "[FLT_SUB]";
												break;
											}
											case modActionTypes::actTy_FLT_MUL:
											{
												doActionPrint = 1;
												actionPrintType = printType::floatPrint;

												lava::INT_FLT_BUNDLE incomingValNum;
												incomingValNum.hex = hexStringToNum(currAction->value);
												lava::INT_FLT_BUNDLE manipNum;
												manipNum.hex = currParamVals.getParamValueNum();
												manipNum.flt *= incomingValNum.flt;
												currParamVals.updateParamValue(manipNum.hex);

												logStream << "[FLT_MUL]";
												std::cout << "[FLT_MUL]";
												break;
											}
											case modActionTypes::actTy_FLT_DIV:
											{
												doActionPrint = 1;
												actionPrintType = printType::floatPrint;

												lava::INT_FLT_BUNDLE incomingValNum;
												incomingValNum.hex = hexStringToNum(currAction->value);
												lava::INT_FLT_BUNDLE manipNum;
												manipNum.hex = currParamVals.getParamValueNum();
												manipNum.flt /= incomingValNum.flt;
												currParamVals.updateParamValue(manipNum.hex);

												logStream << "[FLT_DIV]";
												std::cout << "[FLT_DIV]";
												break;
											}
											case modActionTypes::actTy_FLT_MOD:
											{
												doActionPrint = 1;
												actionPrintType = printType::floatPrint;

												lava::INT_FLT_BUNDLE incomingValNum;
												incomingValNum.hex = hexStringToNum(currAction->value);
												lava::INT_FLT_BUNDLE manipNum;
												manipNum.hex = currParamVals.getParamValueNum();
												manipNum.flt = std::fmod(manipNum.flt, incomingValNum.flt);
												currParamVals.updateParamValue(manipNum.hex);

												logStream << "[FLT_MOD]";
												std::cout << "[FLT_MOD]";
												break;
											}
											case modActionTypes::actTy_BIT_AND:
											{
												doActionPrint = 1;

												unsigned int incomingValNum = hexStringToNum(currAction->value);
												unsigned int manipNum = currParamVals.getParamValueNum();
												manipNum &= incomingValNum;
												currParamVals.updateParamValue(manipNum);

												logStream << "[BIT_AND]";
												std::cout << "[BIT_AND]";
												break;
											}
											case modActionTypes::actTy_BIT_OR:
											{
												doActionPrint = 1;

												unsigned int incomingValNum = hexStringToNum(currAction->value);
												unsigned int manipNum = currParamVals.getParamValueNum();
												manipNum |= incomingValNum;
												currParamVals.updateParamValue(manipNum);

												logStream << "[BIT_OR] ";
												std::cout << "[BIT_OR] ";
												break;
											}
											case modActionTypes::actTy_BIT_XOR:
											{
												doActionPrint = 1;

												unsigned int incomingValNum = hexStringToNum(currAction->value);
												unsigned int manipNum = currParamVals.getParamValueNum();
												manipNum ^= incomingValNum;
												currParamVals.updateParamValue(manipNum);

												logStream << "[BIT_XOR]";
												std::cout << "[BIT_XOR]";
												break;
											}
											case modActionTypes::actTy_BIT_SHIFT_L:
											{
												doActionPrint = 1;

												unsigned int incomingValNum = hexStringToNum(currAction->value);
												unsigned int manipNum = currParamVals.getParamValueNum();
												manipNum = manipNum << incomingValNum;
												currParamVals.updateParamValue(manipNum);

												logStream << "[BIT_SHIFT_L]";
												std::cout << "[BIT_SHIFT_L]";
												break;
											}
											case modActionTypes::actTy_BIT_SHIFT_R:
											{
												doActionPrint = 1;

												unsigned int incomingValNum = hexStringToNum(currAction->value);
												unsigned int manipNum = currParamVals.getParamValueNum();
												manipNum = manipNum >> incomingValNum;
												currParamVals.updateParamValue(manipNum);

												logStream << "[BIT_SHIFT_R]";
												std::cout << "[BIT_SHIFT_R]";
												break;
											}
											case modActionTypes::actTy_BIT_ROTATE_L:
											{
												doActionPrint = 1;

												unsigned int incomingValNum = hexStringToNum(currAction->value);
												unsigned int manipNum = currParamVals.getParamValueNum();
												unsigned int temp = manipNum;
												manipNum = manipNum << incomingValNum;
												temp = temp >> (32 - incomingValNum);
												manipNum |= temp;
												currParamVals.updateParamValue(manipNum);

												logStream << "[BIT_ROTATE_L]";
												std::cout << "[BIT_ROTATE_L]";
												break;
											}
											case modActionTypes::actTy_BIT_ROTATE_R:
											{
												doActionPrint = 1;

												unsigned int incomingValNum = hexStringToNum(currAction->value);
												unsigned int manipNum = currParamVals.getParamValueNum();
												unsigned int temp = manipNum;
												manipNum = manipNum >> incomingValNum;
												temp = temp << (32 - incomingValNum);
												manipNum |= temp;
												currParamVals.updateParamValue(manipNum);

												logStream << "[BIT_ROTATE_R]";
												std::cout << "[BIT_ROTATE_R]";
												break;
											}
											case modActionTypes::actTy_RETARGET_PARAM:
											{
												logStream << "[RETARGET_PARAM]\n";
												std::cout << "[RETARGET_PARAM]\n";

												// Grab the desired param index from the current action
												unsigned int incomingValNum = lava::hexStringToNum(currAction->value);
												// Create a temporary paramTarget which we'll use to test that the targeted param's location valid.
												lava::paramTarget tempTarget = lava::paramTarget(*this, paramOffsetNum, incomingValNum, doingAttributeHack);
												std::size_t tempType = tempTarget.getParamTypeNum();
												std::size_t tempValue = tempTarget.getParamValueNum();
												// If the targeted param's location was valid...
												if (tempType != SIZE_MAX && tempValue != SIZE_MAX)
												{
													//... save the current state of the current target parameter...
													currParamVals.saveParamToContents();
													//... and change our target to the new one.
													currParamVals = tempTarget;

													// Do reporting.
													//We do full reporting here instead of using standard Action reporting since we have extra stuff to report
													logStream << "\t\t\t\tParameter Redirect Triggered: New target is Param Index " << incomingValNum << "\n";
													logStream << "\t\t\t\tParam Val: " << currParamVals.getParamValueString();
													std::cout << "\t\t\t\tParameter Redirect Triggered: New target is Param Index " << incomingValNum << "\n";
													std::cout << "\t\t\t\tParam Val: " << currParamVals.getParamValueString();

													// Set our final print type based on param type, and do sclr/flt print if necessary.
													switch (currParamVals.getParamTypeNum())
													{
														case lava::movesetParamTypes::varTy_SCLR:
														{
															logStream << " (Scalar = " << currParamVals.getParamValueNum() / lava::floatDenominator << ")";
															std::cout << " (Scalar = " << currParamVals.getParamValueNum() / lava::floatDenominator << ")";
															finalPrintType = printType::scalarPrint;
															break;
														}
														case lava::movesetParamTypes::varTy_ATTRIBUTE_FLT:
														{
															logStream << " (Float = " << lava::hexRepToFloat(currParamVals.getParamValueNum()) << ")";
															std::cout << " (Float = " << lava::hexRepToFloat(currParamVals.getParamValueNum()) << ")";
															finalPrintType = printType::floatPrint;
															break;
														}
														default:
														{
															finalPrintType = printType::integerPrint;
															break;
														}
													}
													logStream << " (Redirected)";
													std::cout << " (Redirected)";
												}
												else
												{
													logStream << "\t\t\t\t[ERROR: INDEX LOCATION IS OUT OF BOUNDS]";
													std::cout << "\t\t\t\t[ERROR: INDEX LOCATION IS OUT OF BOUNDS]";
												}
												break;
											}
											case modActionTypes::actTy_CONVERT_PARAM:
											{
												logStream << "[CONVERT_PARAM]";
												std::cout << "[CONVERT_PARAM]";
												unsigned int incomingValNum = lava::hexStringToNum(currAction->value);
												if (incomingValNum < lava::movesetParamTypes::variableTypeCount)
												{
													doActionPrint = 1;
													actionPrintType = printType::integerPrint;
													finalPrintType = printType::integerPrint;
													switch (incomingValNum)
													{
														case lava::movesetParamTypes::varTy_INT:
														{
															if (currParamVals.getParamTypeNum() == lava::movesetParamTypes::varTy_SCLR)
															{
																unsigned int manipNum = currParamVals.getParamValueNum();
																manipNum /= lava::floatDenominator;
																currParamVals.updateParamValue(manipNum);
															}
															break;
														}
														case lava::movesetParamTypes::varTy_SCLR:
														{
															actionPrintType = printType::scalarPrint;
															finalPrintType = printType::scalarPrint;
															if (currParamVals.getParamTypeNum() == lava::movesetParamTypes::varTy_INT)
															{
																unsigned int manipNum = currParamVals.getParamValueNum();
																manipNum *= lava::floatDenominator;
																currParamVals.updateParamValue(manipNum);
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
														case lava::movesetParamTypes::varTy_ATTRIBUTE_INT:
														{
															if (currParamVals.getParamTypeNum() == lava::movesetParamTypes::varTy_ATTRIBUTE_FLT)
															{
																// Initiate a temporary bundle for conversion purposes
																lava::INT_FLT_BUNDLE tempBundle;
																// Set the bundle's hex value to the hex representation of the current val's stored float value
																tempBundle.hex = currParamVals.getParamValueNum();
																// Convert the float representation to the equivalent integer value, then update the param value.
																currParamVals.updateParamValue(int(tempBundle.flt));
															}
															break;
														}
														case lava::movesetParamTypes::varTy_ATTRIBUTE_FLT:
														{
															actionPrintType = printType::floatPrint;
															finalPrintType = printType::floatPrint;
															if (currParamVals.getParamTypeNum() == lava::movesetParamTypes::varTy_ATTRIBUTE_INT)
															{
																// Initiate a temporary bundle for conversion purposes
																lava::INT_FLT_BUNDLE tempBundle;
																// Set the bundle's float to the float representation of the current val's stored integer value
																// This will transform the encoded hex value to float format
																tempBundle.flt = float(currParamVals.getParamValueNum());
																// Update the param with the new hex interpretation.
																currParamVals.updateParamValue(tempBundle.hex);
															}
															break;
														}
														default:
														{
															break;
														}
													}
													currParamVals.updateParamType(incomingValNum);
												}
												else
												{
													logStream << "\t\t\tTarget parameter type (" << incomingValNum << ") was invalid.\n";
													std::cout << "\t\t\tTarget parameter type (" << incomingValNum << ") was invalid.\n";
												}
												break;
											}
											case modActionTypes::actTy_SWAP_PARAMS:
											{
												logStream << "[SWAP_PARAMS]\n";
												std::cout << "[SWAP_PARAMS]\n";

												// Grab the desired param index from the current action
												unsigned int incomingValNum = lava::hexStringToNum(currAction->value);
												// Create a temporary paramTarget which we'll use to test that the targeted param's location valid.
												lava::paramTarget tempTarget = lava::paramTarget(*this, paramOffsetNum, incomingValNum, doingAttributeHack);
												std::size_t tempType = tempTarget.getParamTypeNum();
												std::size_t tempValue = tempTarget.getParamValueNum();
												// If the targeted param's location was valid...
												if (tempType != SIZE_MAX && tempValue != SIZE_MAX)
												{
													// 
													finalPrintType = printType::integerPrint;
													if (tempType == lava::movesetParamTypes::varTy_SCLR)
													{
														finalPrintType = printType::scalarPrint;
													}
													else if (tempType == lava::movesetParamTypes::varTy_ATTRIBUTE_FLT)
													{
														finalPrintType = printType::floatPrint;
													}
													tempTarget.updateParamType(currParamVals.getParamTypeNum());
													tempTarget.updateParamValue(currParamVals.getParamValueNum());
													tempTarget.saveParamToContents();
													currParamVals.updateParamType(tempType);
													currParamVals.updateParamValue(tempValue);
													logStream << "\t\t\t\tParameter Swap Triggered with Param at Index " << hexStringToNum(currAction->value) << "\n";
													std::cout << "\t\t\t\tParameter Swap Triggered with Param at Index " << hexStringToNum(currAction->value) << "\n";
													logStream << "\t\t\t\tThat Param's Type is now: " << tempTarget.getParamTypeNum() << ", and its Value is now: " << tempTarget.getParamValueString();
													std::cout << "\t\t\t\tThat Param's Type is now: " << tempTarget.getParamTypeNum() << ", and its Value is now: " << tempTarget.getParamValueString();

													// Do sclr/flt print if necessary
													switch (tempTarget.getParamTypeNum())
													{
														case lava::movesetParamTypes::varTy_SCLR:
														{
															logStream << " (Scalar = " << tempTarget.getParamValueNum() / lava::floatDenominator << ")";
															std::cout << " (Scalar = " << tempTarget.getParamValueNum() / lava::floatDenominator << ")";
															break;
														}
														case lava::movesetParamTypes::varTy_ATTRIBUTE_FLT:
														{
															logStream << " (Float = " << lava::hexRepToFloat(tempTarget.getParamValueNum()) << ")";
															std::cout << " (Float = " << lava::hexRepToFloat(tempTarget.getParamValueNum()) << ")";
															break;
														}
														default:
														{
															break;
														}
													}

													logStream << "\n";
													std::cout << "\n";


													logStream << "\t\t\t\tThis Param's Type is now: " << currParamVals.getParamTypeNum() << ", and its Value is now: " << currParamVals.getParamValueString();
													std::cout << "\t\t\t\tThis Param's Type is now: " << currParamVals.getParamTypeNum() << ", and its Value is now: " << currParamVals.getParamValueString();

													// Do sclr/flt print if necessary, and set our final print type based on param type.
													switch (currParamVals.getParamTypeNum())
													{
														case lava::movesetParamTypes::varTy_SCLR:
														{
															logStream << " (Scalar = " << currParamVals.getParamValueNum() / lava::floatDenominator << ")";
															std::cout << " (Scalar = " << currParamVals.getParamValueNum() / lava::floatDenominator << ")";
															finalPrintType = printType::scalarPrint;
															break;
														}
														case lava::movesetParamTypes::varTy_ATTRIBUTE_FLT:
														{
															logStream << " (Float = " << lava::hexRepToFloat(currParamVals.getParamValueNum()) << ")";
															std::cout << " (Float = " << lava::hexRepToFloat(currParamVals.getParamValueNum()) << ")";
															finalPrintType = printType::floatPrint;
															break;
														}
														default:
														{
															finalPrintType = printType::integerPrint;
															break;
														}
													}
												}
												else
												{
													logStream << "\t\t\t\t[ERROR: INDEX LOCATION IS OUT OF BOUNDS]";
													std::cout << "\t\t\t\t[ERROR: INDEX LOCATION IS OUT OF BOUNDS]";
												}
												break;
											}
											default:
											{
												logStream << "[INVALID_TYPE]";
												std::cout << "[INVALID_TYPE]";
												break;
											}
										}

										if (doActionPrint)
										{
											switch (actionPrintType)
											{
												case printType::scalarPrint:
												{
													logStream << " Value = " << currAction->value;
													std::cout << " Value = " << currAction->value;
													float incomingFlt = hexStringToNum(currAction->value) / lava::floatDenominator;
													float intermediateFlt = lava::hexStringToNum(intermediateParamValString) / lava::floatDenominator;
													logStream << " (Scalar = " << incomingFlt << ")";
													std::cout << " (Scalar = " << incomingFlt << ")";
													logStream << "\n\t\t\t\t" << intermediateParamValString << " (" << intermediateFlt << ") -> " << currParamVals.getParamValueString() << " (" << currParamVals.getParamValueNum() / lava::floatDenominator << ")\n";
													std::cout << "\n\t\t\t\t" << intermediateParamValString << " (" << intermediateFlt << ") -> " << currParamVals.getParamValueString() << " (" << currParamVals.getParamValueNum() / lava::floatDenominator << ")\n";
													break;
												}
												case printType::floatPrint:
												{
													logStream << " Value = " << currAction->value;
													std::cout << " Value = " << currAction->value;
													float incomingFlt = lava::hexRepToFloat(hexStringToNum(currAction->value));
													float intermediateFlt = lava::hexRepToFloat(lava::hexStringToNum(intermediateParamValString));
													float currParamFlt = lava::hexRepToFloat(currParamVals.getParamValueNum());
													logStream << " (Float = " << incomingFlt << ")";
													std::cout << " (Float = " << incomingFlt << ")";
													logStream << "\n\t\t\t\t" << intermediateParamValString << " (" << intermediateFlt << ") -> " << currParamVals.getParamValueString() << " (" << currParamFlt << ")\n";
													std::cout << "\n\t\t\t\t" << intermediateParamValString << " (" << intermediateFlt << ") -> " << currParamVals.getParamValueString() << " (" << currParamFlt << ")\n";
													break;
												}
												default:
												{
													logStream << " Value = " << currAction->value << "\n\t\t\t\t" << intermediateParamValString << " -> " << currParamVals.getParamValueString() << "\n";
													std::cout << " Value = " << currAction->value << "\n\t\t\t\t" << intermediateParamValString << " -> " << currParamVals.getParamValueString() << "\n";
													break;
												}
											}
										}
										else
										{
											std::cout << "\n";
											logStream << "\n";
										}
									}

									// Handle lock
									bool lockUsed = 0;
									std::size_t lockHexNum = lava::hexStringToNum(currMod->locked);
									std::size_t lockBufferForCanonVal = lava::hexStringToNum(canonParamValString);
									lockBufferForCanonVal &= lockHexNum;
									std::size_t lockBufferForCurrVal = currParamVals.getParamValueNum();
									lockBufferForCurrVal &= ~(lockHexNum);
									lockBufferForCurrVal |= lockBufferForCanonVal;
									lockUsed = lockBufferForCurrVal != currParamVals.getParamValueNum();
									currParamVals.updateParamValue(lockBufferForCurrVal);
									
									// Only print the result of the lock operation if the value changed as a result of applying it.
									if (lockUsed)
									{
										std::cout << "\t\t\tLock (" << currMod->locked << "): " << currParamVals.getParamValueString() << "\n";
										logStream << "\t\t\tLock (" << currMod->locked << "): " << currParamVals.getParamValueString() << "\n";
									}

									// Write result to contents.
									currParamVals.saveParamToContents();

									if (currMod->actions.size())
									{
										// Report results:
										std::string finalValueString = "";
										if (doingAttributeHack)
										{
											finalValueString = lava::hexVecToHexStringWithPadding(
												contents.getBytes(4, currParamVals.getParamOffsetNum() + currParamVals.getParamIndexOffset(), numGotten),
												lava::canonParamLengthStr
											);
										}
										else
										{
											finalValueString = lava::hexVecToHexStringWithPadding(
												contents.getBytes(4, currParamVals.getParamOffsetNum() + currParamVals.getParamIndexOffset() + 4, numGotten),
												lava::canonParamLengthStr
											);
										}
										logStream << "\t\t\tFinal Value: " << finalValueString;
										std::cout << "\t\t\tFinal Value: " << finalValueString;

										switch (finalPrintType)
										{
											case printType::scalarPrint:
											{
												logStream << " (Scalar = " << currParamVals.getParamValueNum() / lava::floatDenominator << ")";
												std::cout << " (Scalar = " << currParamVals.getParamValueNum() / lava::floatDenominator << ")";
												break;
											}
											case printType::floatPrint:
											{
												logStream << " (Float = " << lava::hexRepToFloat(currParamVals.getParamValueNum()) << ")";
												std::cout << " (Float = " << lava::hexRepToFloat(currParamVals.getParamValueNum()) << ")";
												break;
											}
											default:
											{
												break;
											}
										}
										logStream << "\n";
										std::cout << "\n";
									}
									logStream << "\n";
									std::cout << "\n";
								}
								else
								{
									previousModUsed = 0;
								}
							}

							// Prints a new line if no matches were reported, just keeps logs pretty
							if (!matchFound)
							{
								logStream << "\n";
								std::cout << "\n";
							}
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
		std::cout << "\n\"" << patchIn.name << "\" Patch Results Summary:\n";
		logStream << "\n\"" << patchIn.name << "\" Patch Results Summary:\n";
		// Iterate through each target:
		for (std::size_t i = 0; i < targetCount; i++)
		{
			logStream << "\t\"" << patchIn.targets[i].name << "\":\n";
			std::cout << "\t\"" << patchIn.targets[i].name << "\":\n";
			for (std::size_t modItr = 0; modItr < modificationCount; modItr++)
			{
				logStream << "\t\t\"" << patchIn.modifications[modItr].name << "\"";
				std::cout << "\t\t\"" << patchIn.modifications[modItr].name << "\"";
				std::size_t appliedCount = results.timesTargetsHit[i][modItr];
				if (appliedCount == 0)
				{
					logStream << " was never applied.\n";
					std::cout << " was never applied.\n";
				}
				else
				{
					logStream << " applied " << appliedCount << " time(s)\n";
					std::cout << " applied " << appliedCount << " time(s)\n";
				}
			}
		}
		std::cout << "\n";
		logStream << "\n";

		return results;
	}

	paramTarget::paramTarget() {};
	paramTarget::paramTarget(movesetFile& parent, std::size_t paramOffsetIn, int paramIndexIn, bool attributeModeIn)
	{
		if (parent.contents.size)
		{
			parentPtr = &parent;
			if (attributeModeIn)
			{
				targetParamIndex = paramIndexIn;
				targetParamIndexOffset = (4 * targetParamIndex);
				updateParamOffset(paramOffsetIn);
				std::size_t numGotten;
				std::vector<char> tempByteVec = { 0, 0, 0, 0 };
				tempByteVec = parentPtr->contents.getBytes(4, this->paramOffsetNum + targetParamIndexOffset, numGotten);
				if (numGotten == 4)
				{
					updateParamValue(tempByteVec);
				}
				if ((paramValueNum & 0x7FFFFFFF) < 0x10000000)
				{
					updateParamType(lava::movesetParamTypes::varTy_ATTRIBUTE_INT);
				}
				else
				{
					updateParamType(lava::movesetParamTypes::varTy_ATTRIBUTE_FLT);
				}
				attributeMode = attributeModeIn;
			}
			else
			{
				targetParamIndex = paramIndexIn;
				targetParamIndexOffset = (8 * targetParamIndex);
				updateParamOffset(paramOffsetIn);
				std::size_t numGotten;
				std::vector<char> tempByteVec = { 0, 0, 0, 0 };
				tempByteVec = parentPtr->contents.getBytes(4, this->paramOffsetNum + targetParamIndexOffset, numGotten);
				if (numGotten == 4)
				{
					updateParamType(tempByteVec);
				}
				tempByteVec = parentPtr->contents.getBytes(4, this->paramOffsetNum + targetParamIndexOffset + 4, numGotten);
				if (numGotten == 4)
				{
					updateParamValue(tempByteVec);
				}
			}
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
		if (!attributeMode)
		{
			paramTypeIdentifierNum = paramTypeIdentifierNumIn;
			paramTypeIdentifier = lava::numToHexVec(paramTypeIdentifierNum);
		}
	}
	void paramTarget::updateParamType(std::vector<char> paramTypeIdentifierIn)
	{
		if (!attributeMode)
		{
			paramTypeIdentifier = paramTypeIdentifierIn;
			paramTypeIdentifierNum = lava::hexVecToNum(paramTypeIdentifier);
		}
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
		bool result = 0;
		if (parentPtr != nullptr)
		{
			result = 1;
			if (attributeMode)
			{
				result &= parentPtr->contents.setBytes(paramValue, paramOffsetNum + targetParamIndexOffset);
			}
			else
			{
				result &= parentPtr->contents.setBytes(paramTypeIdentifier, paramOffsetNum + targetParamIndexOffset);
				result &= parentPtr->contents.setBytes(paramValue, paramOffsetNum + targetParamIndexOffset + 4);
			}
		}
		return result;
	}
}

