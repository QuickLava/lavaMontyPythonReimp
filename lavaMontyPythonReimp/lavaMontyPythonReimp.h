#ifndef LAVA_MONTY_PYTHON_REIMP_H_V1
#define LAVA_MONTY_PYTHON_REIMP_H_V2

#include <conio.h>
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <iomanip>
#include "pugi/pugixml.hpp"

namespace lava
{
	std::string stringToUpper(const std::string& stringIn);
	std::string sanitizeHexStrInput(const std::string& stringIn, bool XAllowed = 0);
	
	std::size_t hexVecToNum(const std::vector<char>& vecIn);
	std::vector<char> numToHexVec(std::size_t vecIn);
	std::size_t hexStringToNum(const std::string& stringIn);
	std::string numToHexStringWithPadding(std::size_t numIn, std::size_t paddingLength);
	std::string numToDecStringWithPadding(std::size_t numIn, std::size_t paddingLength);
	bool hexStrComp(const std::string& str1, const std::string& str2);
	std::vector<std::pair<std::string, std::string>> createValuePairs(std::ifstream& fileIn);
	void addVectors(std::vector<std::size_t> &vec1, const std::vector<std::size_t> &vec2);

	struct movesetPatch;
	std::vector<movesetPatch> parseMovesetPatchXML(std::string fileIn);

	constexpr std::size_t canonParamLengthStr = 8;
	constexpr std::size_t canonParamLengthInBytes = canonParamLengthStr / 2;
	constexpr std::size_t globalBaseOffset = 0x80;
	constexpr float floatDenominator = 0xEA60;
	const std::string changelogSuffix = "_changelog.txt";

	/*std::vector<char> stringToVec(const std::string& stringIn);
	std::string vecToString(const std::vector<char>& vecIn);*/

	struct byteArray
	{
		std::size_t size = 0;
		std::vector<char> body = {};
		void populate(std::istream& sourceStream);
		std::vector<char> getBytes(std::size_t numToGet, std::size_t startIndex, std::size_t& numGot);
		bool setBytes(std::vector<char> bytesIn, std::size_t atIndex);
		std::size_t search(const std::vector<char>& searchCriteria, std::size_t startItr = 0, std::size_t endItr = SIZE_MAX);
		std::vector<std::size_t> searchMultiple(const std::vector<char>& searchCriteria, std::size_t startItr = 0, std::size_t endItr = SIZE_MAX);
		bool dumpToFile(std::string targetPath);
	};

	struct movesetPatchTarget
	{
		std::string name = "";
		std::vector<char> signature = {};
		int paramIndex = INT_MAX;
	};
	enum movesetVarTypes
	{
		varTy_INT = 0,
		varTy_SCLR,
		varTy_PNTR,
		varTy_BOOL,
		variableTypeCount
	};
	enum modActionTypes
	{
		actTy_NULL = -1,
		actTy_DO_NOTHING = 0,
		actTy_REPLACE,
		actTy_ADD,
		actTy_SUB,
		actTy_MUL,
		actTy_DIV,
		actTy_BIT_AND,
		actTy_BIT_OR,
		actTy_BIT_XOR,
		actTy_RETARGET_PARAM,
		actionTypeCount
	};
	struct movesetPatchModAction
	{
		int actionType = modActionTypes::actTy_NULL;
		std::string value = "";
	};
	struct movesetPatchMod
	{
		std::string match = "FFFFFFFF";
		std::string locked = "00000000";
		int redirect = INT_MAX;
		std::vector<movesetPatchModAction> actions;
	};
	struct movesetPatch
	{
		std::string name = "";
		std::vector<movesetPatchTarget> targets = {};
		std::vector<movesetPatchMod> modifications = {};
	};
	struct movesetPatchResult
	{
		std::vector<std::vector<std::size_t>> timesTargetsHit;
	};

	struct movesetFile
	{
	private:
	public:

		std::size_t movesetLength = 0;
		std::size_t dataLength = 0;
		std::size_t dataOffset = 0;
		std::size_t offsetSectionCount = 0;
		std::size_t offsetSectionOffset = 0;
		std::size_t dataTableCount = 0;
		std::size_t dataTableOffset = 0;
		std::size_t externalDataCount = 0;
		std::size_t externalDataOffset = 0;
		std::size_t tablesEnd = 0;
		std::string filePath = "";

		byteArray contents;
		bool init(std::string filePathIn);
		std::string fetchString(std::size_t strAddr);
		void summarizeTable(std::size_t tableAddr, std::size_t entryCount, std::size_t offsetShiftSize, std::string prefix = "\t\t", std::ostream& output = std::cout);
		std::vector<std::size_t> changeMatchingParameter(std::vector<std::pair<std::string, std::vector<char>>> functions, std::vector<std::pair<std::string, std::string>> matches, std::size_t parameterOffsetInBytes);
		movesetPatchResult applyMovesetPatch(const movesetPatch& patchIn);


	};

}

std::ostream& operator<<(std::ostream& out, const std::vector<char>& in);


#endif