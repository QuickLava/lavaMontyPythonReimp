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
#include <map>
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
		int paramType = INT_MAX;
	};
	enum movesetParamTypes
	{
		varTy_INT = 0,
		varTy_SCLR,
		varTy_PNTR,
		varTy_BOOL,
		varTy_4,
		varTy_VAR,
		varTy_REQ,
		variableTypeCount
	};
	enum modActionTypes
	{
		actTy_NULL = -1,
		// Simple Block
		actTy_DO_NOTHING = 0x00,
		actTy_REPLACE,
		// A is for Arithmetic
		actTy_INT_ADD = 0xA0,
		actTy_INT_SUB,
		actTy_INT_MUL,
		actTy_INT_DIV, 
		actTy_FLT_ADD,
		actTy_FLT_SUB,
		actTy_FLT_MUL,
		actTy_FLT_DIV,
		// B is for Bit Manipulation
		actTy_BIT_AND = 0xB0,
		actTy_BIT_OR,
		actTy_BIT_XOR,
		actTy_BIT_SHIFT_L,
		actTy_BIT_SHIFT_R,
		actTy_BIT_ROTATE_L,
		actTy_BIT_ROTATE_R,
		// C is for CAUTION
		// You should't ever need to do this, use mod redirects isntead
		actTy_RETARGET_PARAM = 0xC0,
		actTy_CONVERT_PARAM,
		//neo_actTy_SWAP_PARAMS,
	};
	enum matchEvaluationMethod
	{
		mtEvl_EQUALS = 0,
		mtEvl_NOT_EQUALS,
		mtEvl_GREATER,
		mtEvl_GREATER_OE,
		mtEvl_LESSER,
		mtEvl_LESSER_OE,
		mtEvl_BIT_AND,
		mtEvl_BIT_XOR,
		evaluationMethodCount
	};

	struct movesetPatchModAction
	{
		int actionType = modActionTypes::actTy_NULL;
		std::string value = "";
	};

	enum extraConditionTypes
	{
		exCon_NULL = 0,
		exCon_PREV_USED,
		exCon_PREV_NOT_USED,
		extraConditionTypeCount
	};
	struct movesetPatchMod
	{
		std::string name = "";
		std::string match = "FFFFFFFF";
		int matchMethod = matchEvaluationMethod::mtEvl_EQUALS;
		std::string locked = "00000000";
		int paramIndexRedirect = INT_MAX;
		int extraCondition = extraConditionTypes::exCon_NULL;
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
		void summarizeOffsetSection(std::ostream& output = std::cout, std::size_t adjustment = 0x80);

		std::vector<std::size_t> changeMatchingParameter(std::vector<std::pair<std::string, std::vector<char>>> functions, std::vector<std::pair<std::string, std::string>> matches, std::size_t parameterOffsetInBytes);

		
		movesetPatchResult applyMovesetPatch(const movesetPatch& patchIn);
	};

	struct paramTarget
	{
	private:
		movesetFile* parentPtr = nullptr;

		int targetParamIndex = 0;
		std::size_t targetParamIndexOffset = SIZE_MAX;

		std::vector<char> paramOffset = { 0, 0, 0, 0 };
		std::size_t paramOffsetNum = SIZE_MAX;

		std::vector<char> paramTypeIdentifier = { 0, 0, 0, 0 };
		std::size_t paramTypeIdentifierNum = SIZE_MAX;

		std::vector<char> paramValue = { 0, 0, 0, 0 };
		std::size_t paramValueNum = SIZE_MAX;
		std::string paramValueString = "00000000";

	public:
		paramTarget();
		paramTarget(movesetFile& parent, std::size_t paramOffsetIn, int paramIndexIn);

		int getParamIndex();
		std::size_t getParamIndexOffset();

		std::size_t getParamOffsetNum();
		std::vector<char> getParamOffset();

		std::size_t getParamTypeNum();
		std::vector<char> getParamType();

		std::size_t getParamValueNum();
		std::string getParamValueString();
		std::vector<char> getParamValue();

		void updateParamOffset(std::size_t paramOffsetNumIn);
		void updateParamOffset(std::vector<char> paramOffsetIn);

		void updateParamType(std::size_t paramTypeIdentifierNumIn);
		void updateParamType(std::vector<char> paramTypeIdentifierIn);

		void updateParamValue(std::size_t paramValueNumIn);
		void updateParamValue(std::string paramValueStringIn);
		void updateParamValue(std::vector<char> paramValueIn);

		bool saveParamToContents();
	};
}

std::ostream& operator<<(std::ostream& out, const std::vector<char>& in);


#endif