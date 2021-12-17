#include "lavaMontyPythonReimp.h"

namespace lava
{
	std::ofstream changelogStream = std::ofstream();

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
									currMod.match = patchModAttr.as_string();
								}
								else if (patchModAttr.name() == "match")
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
											currMod.matchMethod = lava::matchEvaluationMethodTypes::mtEvl_EQUALS;
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
							modNumber++;
							currPatch.modifications.push_back(currMod);
						}
					}
					else if (patchChild.name() == "directApply")
					{
						std::string baseName = "DirectApply";
						unsigned int templateAction = lava::modActionTypes::actTy_REPLACE;
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
								std::string manipStr = directApplyAttr.as_string();
								if (manipStr.find("0x") == 0)
								{
									manipStr = manipStr.substr(2, std::string::npos);
									templateAction = std::stoi(manipStr, nullptr, 16);
								}
								else
								{
									templateAction = directApplyAttr.as_int(INT_MAX);
									if (templateAction < 0)
									{
										templateAction = INT_MAX;
									}
								}
							}
							else if (directApplyAttr.name() == "paramIndexRedirect")
							{
								std::string manipStr = directApplyAttr.as_string();
								if (manipStr.find("0x") == 0)
								{
									manipStr = manipStr.substr(2, std::string::npos);
									templateRedirectParam = std::stoi(manipStr, nullptr, 16);
								}
								else
								{
									templateRedirectParam = directApplyAttr.as_int(INT_MAX);
									if (templateRedirectParam < 0)
									{
										templateRedirectParam = INT_MAX;
									}
								}
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
													tempMod.match = match;
													tempMod.paramIndexRedirect = templateRedirectParam;
													lava::movesetPatchModAction tempAction;
													tempAction.actionType = templateAction;
													tempAction.value = value;
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

		// Loop through every target
		for (std::size_t targetItr = 0; targetItr < targetCount; targetItr++)
		{
			// Grab pointer to current target, and ping for its signature within the data section
			const movesetPatchTarget* currTarget = &patchIn.targets[targetItr];
			std::cout << "\"" << currTarget->name << "\" [0x" << lava::hexVecToHexStringWithPadding(currTarget->signature, lava::canonParamLengthStr) << "] (Param " << currTarget->paramIndex;
			logStream << "\"" << currTarget->name << "\" [0x" << lava::hexVecToHexStringWithPadding(currTarget->signature, lava::canonParamLengthStr) << "] (Param " << currTarget->paramIndex;
			if (currTarget->paramType != INT_MAX)
			{
				std::cout << ", of Type " << currTarget->paramType;
				logStream << ", of Type " << currTarget->paramType;
			}
			std::cout << "):\n";
			logStream << "):\n";
			
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

					// Determine whether or not that's actually a valid location or not, and if so, proceed.
					if (paramOffsetNum < (dataOffset + dataLength))
					{
						// Create our param target. This holds the current value of the target parameter, as well as where it's stored in the file, and is the variable that we'll be modifying to make changes to the parameter's value and type.
						lava::paramTarget currParamVals(*this, paramOffsetNum, currTarget->paramIndex);

						// Process target paramType matching. If no type was specified, or if one was specified and it matches our found type, proceed.
						if (currTarget->paramType == INT_MAX || currParamVals.getParamTypeNum() == currTarget->paramType)
						{
							// Report location of current ping, as well as its paramOffset. Note, the first paramOffset we report is non .pac adjusted since that's the one you'll see in PSAC, which is what I expect most people will be looking for. The adjusted value is provided second.
							logStream << "\t[0x" << numToHexStringWithPadding(currPing, 8) << "]: Params @ 0x" << numToHexStringWithPadding(paramOffsetNum - dataOffset, 8);
							std::cout << "\t[0x" << numToHexStringWithPadding(currPing, 8) << "]: Params @ 0x" << numToHexStringWithPadding(paramOffsetNum - dataOffset, 8);

							// Initialize values for loop
							bool redirectUsed = 0;
							bool previousModUsed = 0;
							std::size_t modItr = 0;
							bool matchFound = 0;
							bool doScalarActionPrint = 0;
							bool doScalarFinalPrint = 0;
							const movesetPatchMod* currMod = nullptr;

							// Do initial param val reporting
							logStream << ", Param Val: " << currParamVals.getParamValueString();
							std::cout << ", Param Val: " << currParamVals.getParamValueString();
							if (currParamVals.getParamTypeNum() == lava::movesetParamTypes::varTy_SCLR)
							{
								logStream << " (Scalar = " << currParamVals.getParamValueNum() / lava::floatDenominator << ")";
								std::cout << " (Scalar = " << currParamVals.getParamValueNum() / lava::floatDenominator << ")";
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
									currParamVals = lava::paramTarget(*this, paramOffsetNum, currMod->paramIndexRedirect);
									// Record that the redirect happened (used for reporting later).
									redirectUsed = 1;
								}
								// Otherwise...
								else
								{
									//... ensure we're on the parameter originally specified by the target.
									currParamVals = lava::paramTarget(*this, paramOffsetNum, currTarget->paramIndex);
									// Record that no redirect happened.
									redirectUsed = 0;
								}

								// Initiate match evaluation result variable
								bool matchEvalRes = 0;
								// Build the version of the match string we'll be using. This just replaces any passthrough chars ('X's) with the digit in the corresponding slot of the target param, ensuring they're equal.
								std::string tempEvalStr = currMod->match;
								for (std::size_t i = 0; i < tempEvalStr.size(); i++)
								{
									if (tempEvalStr[i] == 'X')
									{
										tempEvalStr[i] = currParamVals.getParamValueString()[i];
									}
								}
								// Actually evaluate the various methods.
								switch (currMod->matchMethod)
								{
									case lava::matchEvaluationMethodTypes::mtEvl_EQUALS:
									{
										matchEvalRes = hexStrComp(tempEvalStr, currParamVals.getParamValueString());
										break;
									}
									case lava::matchEvaluationMethodTypes::mtEvl_NOT_EQUALS:
									{
										matchEvalRes = !hexStrComp(tempEvalStr, currParamVals.getParamValueString());
										break;
									}
									case lava::matchEvaluationMethodTypes::mtEvl_GREATER:
									{
										matchEvalRes = lava::hexStringToNum(tempEvalStr) > currParamVals.getParamValueNum();
										break;
									}
									case lava::matchEvaluationMethodTypes::mtEvl_GREATER_OE:
									{
										matchEvalRes = lava::hexStringToNum(tempEvalStr) >= currParamVals.getParamValueNum();
										break;
									}
									case lava::matchEvaluationMethodTypes::mtEvl_LESSER:
									{
										matchEvalRes = lava::hexStringToNum(tempEvalStr) < currParamVals.getParamValueNum();
										break;
									}
									case lava::matchEvaluationMethodTypes::mtEvl_LESSER_OE:
									{
										matchEvalRes = lava::hexStringToNum(tempEvalStr) <= currParamVals.getParamValueNum();
										break;
									}
									case lava::matchEvaluationMethodTypes::mtEvl_BIT_AND:
									{
										matchEvalRes = lava::hexStringToNum(tempEvalStr) & currParamVals.getParamValueNum();
										break;
									}
									case lava::matchEvaluationMethodTypes::mtEvl_BIT_XOR:
									{
										matchEvalRes = lava::hexStringToNum(tempEvalStr) ^ currParamVals.getParamValueNum();
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
									switch (currMod->matchMethod)
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
										case lava::matchEvaluationMethodTypes::mtEvl_BIT_XOR:
										{
											std::cout << "^";
											logStream << "^";
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
											std::cout << " [REQ_PREV]\n";
											logStream << " [REQ_PREV]\n";
											break;
										}
										case lava::extraConditionTypes::exCon_AND_PREV_NOT_USED:
										{
											std::cout << " [REQ_NOT_PREV]\n";
											logStream << " [REQ_NOT_PREV]\n";
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
									bool actionOccured = 0;

									// Iterate through each action:
									for (std::size_t actionItr = 0; actionItr < currMod->actions.size(); actionItr++)
									{
										// Store intermediate paramString
										intermediateParamValString = currParamVals.getParamValueString();
										// Reset action tracker
										actionOccured = 0;
										doScalarActionPrint = 0;
										doScalarFinalPrint = 0;

										// Do changelog reporting
										logStream << "\t\t\tAction #" << actionItr << ": ";
										std::cout << "\t\t\tAction #" << actionItr << ": ";
										currAction = &currMod->actions[actionItr];
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
											actionOccured = 1;

											currParamVals.updateParamValue(currAction->value);

											logStream << "[REP]";
											std::cout << "[REP]";
											break;
										}
										case modActionTypes::actTy_INT_ADD:
										{
											actionOccured = 1;

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
											actionOccured = 1;

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
											actionOccured = 1;

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
											actionOccured = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = currParamVals.getParamValueNum();
											manipNum /= incomingValNum;
											currParamVals.updateParamValue(manipNum);

											logStream << "[INT_DIV]";
											std::cout << "[INT_DIV]";
											break;
										}
										case modActionTypes::actTy_FLT_ADD:
										{
											actionOccured = 1;
											doScalarActionPrint = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = currParamVals.getParamValueNum();
											manipNum += incomingValNum;
											currParamVals.updateParamValue(manipNum);

											logStream << "[FLT_ADD]";
											std::cout << "[FLT_ADD]";
											break;
										}
										case modActionTypes::actTy_FLT_SUB:
										{
											actionOccured = 1;
											doScalarActionPrint = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = currParamVals.getParamValueNum();
											manipNum -= incomingValNum;
											currParamVals.updateParamValue(manipNum);

											logStream << "[FLT_SUB]";
											std::cout << "[FLT_SUB]";
											break;
										}
										case modActionTypes::actTy_FLT_MUL:
										{
											actionOccured = 1;
											doScalarActionPrint = 1;

											float incomingValNum = hexStringToNum(currAction->value) / lava::floatDenominator;
											unsigned int manipNum = currParamVals.getParamValueNum();
											manipNum *= incomingValNum;
											currParamVals.updateParamValue(manipNum);

											logStream << "[FLT_MUL]";
											std::cout << "[FLT_MUL]";
											break;
										}
										case modActionTypes::actTy_FLT_DIV:
										{
											actionOccured = 1;
											doScalarActionPrint = 1;

											float incomingValNum = hexStringToNum(currAction->value) / lava::floatDenominator;
											unsigned int manipNum = currParamVals.getParamValueNum();
											manipNum /= incomingValNum;
											currParamVals.updateParamValue(manipNum);

											logStream << "[FLT_DIV]";
											std::cout << "[FLT_DIV]";
											break;
										}
										case modActionTypes::actTy_BIT_AND:
										{
											actionOccured = 1;

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
											actionOccured = 1;

											unsigned int incomingValNum = hexStringToNum(currAction->value);
											unsigned int manipNum = currParamVals.getParamValueNum();
											manipNum |= incomingValNum;
											currParamVals.updateParamValue(manipNum);

											logStream << "[BIT_OR]";
											std::cout << "[BIT_OR]";
											break;
										}
										case modActionTypes::actTy_BIT_XOR:
										{
											actionOccured = 1;

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
											actionOccured = 1;

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
											actionOccured = 1;

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
											actionOccured = 1;

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
											actionOccured = 1;

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
											if (hexStringToNum(currAction->value) >= 0)
											{
												actionOccured = 1;
												currParamVals.saveParamToContents();
												currParamVals = lava::paramTarget(*this, paramOffsetNum, lava::hexStringToNum(currAction->value));
												logStream << "[TARGET_PARAM]\n";
												std::cout << "[TARGET_PARAM]\n";
												logStream << "\t\t\tParameter Redirect Triggered: New target is Param Index " << hexStringToNum(currAction->value) << "\n";
												logStream << "\t\t\tParam Val: " << currParamVals.getParamValueString();
												std::cout << "\t\t\tParameter Redirect Triggered: New target is Param Index " << hexStringToNum(currAction->value) << "\n";
												std::cout << "\t\t\tParam Val: " << currParamVals.getParamValueString();
												if (currParamVals.getParamTypeNum() == lava::movesetParamTypes::varTy_SCLR)
												{
													logStream << " (Scalar = " << currParamVals.getParamValueNum() / lava::floatDenominator << ")";
													std::cout << " (Scalar = " << currParamVals.getParamValueNum() / lava::floatDenominator << ")";
												}
												logStream << " (Redirected)\n";
												std::cout << " (Redirected)\n";
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
													doScalarActionPrint = 1;
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
										}
										default:
										{
											logStream << "[INVALID_TYPE]";
											std::cout << "[INVALID_TYPE]";
											break;
										}
										}

										if (actionOccured)
										{
											if (currParamVals.getParamTypeNum() == lava::movesetParamTypes::varTy_SCLR || doScalarActionPrint)
											{
												logStream << " Value = " << currAction->value;
												std::cout << " Value = " << currAction->value;
												float incomingFlt = hexStringToNum(currAction->value) / lava::floatDenominator;
												logStream << " (Scalar = " << incomingFlt << ")";
												std::cout << " (Scalar = " << incomingFlt << ")";
												float intermediateFlt = lava::hexStringToNum(intermediateParamValString) / lava::floatDenominator;
												logStream << "\n\t\t\t\t" << intermediateParamValString << " (" << intermediateFlt << ") -> " << currParamVals.getParamValueString() << " (" << currParamVals.getParamValueNum() / lava::floatDenominator << ")\n";
												std::cout << "\n\t\t\t\t" << intermediateParamValString << " (" << intermediateFlt << ") -> " << currParamVals.getParamValueString() << " (" << currParamVals.getParamValueNum() / lava::floatDenominator << ")\n";
											}
											else
											{
												logStream << " Value = " << currAction->value << "\n\t\t\t\t" << intermediateParamValString << " -> " << currParamVals.getParamValueString() << "\n";
												std::cout << " Value = " << currAction->value << "\n\t\t\t\t" << intermediateParamValString << " -> " << currParamVals.getParamValueString() << "\n";
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
									for (std::size_t i = 0; i < 8; i++)
									{
										std::string lockApplicationStr = currParamVals.getParamValueString();
										if (currMod->locked[i] == '1' && lockApplicationStr[i] != canonParamValString[i])
										{
											lockApplicationStr[i] = canonParamValString[i];
											lockUsed = 1;
										}
										currParamVals.updateParamValue(lockApplicationStr);
									}
									// Only print the result of the lock operation if the value changed as a result of applying it.
									if (lockUsed)
									{
										std::cout << "\t\t\tLock (" << currMod->locked << "): " << currParamVals.getParamValueString() << "\n";
										logStream << "\t\t\tLock (" << currMod->locked << "): " << currParamVals.getParamValueString() << "\n";
									}

									// Write result to contents.
									currParamVals.saveParamToContents();

									// Report results:
									logStream << "\t\t\tFinal Value: " << lava::hexVecToHexStringWithPadding(contents.getBytes(4, currParamVals.getParamOffsetNum() + currParamVals.getParamIndexOffset() + 4, numGotten), lava::canonParamLengthStr);
									std::cout << "\t\t\tFinal Value: " << lava::hexVecToHexStringWithPadding(contents.getBytes(4, currParamVals.getParamOffsetNum() + currParamVals.getParamIndexOffset() + 4, numGotten), lava::canonParamLengthStr);
									// Do scalar report if necessary
									if (currParamVals.getParamTypeNum() == lava::movesetParamTypes::varTy_SCLR || doScalarFinalPrint)
									{
										logStream << " (Scalar = " << currParamVals.getParamValueNum() / lava::floatDenominator << ")";
										std::cout << " (Scalar = " << currParamVals.getParamValueNum() / lava::floatDenominator << ")";
									}
									logStream << "\n\n";
									std::cout << "\n\n";
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

