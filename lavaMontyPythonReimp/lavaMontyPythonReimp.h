#ifndef LAVA_MONTY_PYTHON_REIMP_H_V1
#define LAVA_MONTY_PYTHON_REIMP_H_V2

#include "lavaMPRUtility.h"

namespace lava
{
	extern std::ofstream changelogStream;
	constexpr std::size_t canonParamLengthStr = 8;
	constexpr std::size_t canonParamLengthInBytes = canonParamLengthStr / 2;
	constexpr std::size_t PACFileHeaderLength = 0x60;
	constexpr std::size_t movesetHeaderLength = 0x20;
	constexpr float floatDenominator = 0xEA60;
	constexpr float attributefloatDenominator = 0x3F800000;
	constexpr std::size_t canonAttributeSectionLength = 0x2E4;
	constexpr std::size_t canonAttributeLengthInBytes = 0x4;
	constexpr std::size_t canonAttributeSectionCount = canonAttributeSectionLength/canonAttributeLengthInBytes;
	const std::string changelogSuffix = "_changelog.txt";
	const std::string validHexChars = "0123456789ABCDEF";

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
		actTy_DO_NOTHING = -1,
		// Simple Block
		actTy_REPLACE = 0x00,
		actTy_RANDOM,
		// A is for Arithmetic
		actTy_INT_ADD = 0xA0,
		actTy_INT_SUB,
		actTy_INT_MUL,
		actTy_INT_DIV,
		actTy_INT_MOD,
		actTy_SCLR_ADD,
		actTy_SCLR_SUB,
		actTy_SCLR_MUL,
		actTy_SCLR_DIV,
		actTy_SCLR_MOD,
		// B is for Bit Manipulation
		actTy_BIT_AND = 0xB0,
		actTy_BIT_OR,
		actTy_BIT_XOR,
		actTy_BIT_SHIFT_L,
		actTy_BIT_SHIFT_R,
		actTy_BIT_ROTATE_L,
		actTy_BIT_ROTATE_R,
		// F is for CAUTION (lol)
		// You should't often have need for any of these, but they're here.
		actTy_RETARGET_PARAM = 0xF0,
		actTy_CONVERT_PARAM,
		actTy_SWAP_PARAMS,
	};
	enum matchEvaluationMethodTypes
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
	enum extraConditionTypes
	{
		exCon_NULL = 0,
		exCon_AND_PREV_USED,
		exCon_AND_PREV_NOT_USED,
		exCon_OR_PREV_USED,
		exCon_OR_PREV_NOT_USED,
		exCon_XOR_PREV_USED,
		exCon_XOR_PREV_NOT_USED,
		extraConditionTypeCount
	};
	
	bool isHexChar(char charIn);
	std::string sanitizeHexStrInput(const std::string& stringIn, bool XAllowed = 0);
	int parseXMLValueAsNum(const std::string& stringIn, bool allowNeg = 0, int defaultVal = INT_MAX);
	bool hexStrComp(const std::string& str1, const std::string& str2);

	struct movesetPatch;
	std::vector<movesetPatch> parseMovesetPatchXML(std::string fileIn);

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
	struct movesetPatchModAction
	{
		std::string name = "";
		int actionType = modActionTypes::actTy_DO_NOTHING;
		std::string value = "";
	};
	struct movesetPatchMod
	{
		std::string name = "";
		std::string match = "FFFFFFFF";
		int evalMethod = matchEvaluationMethodTypes::mtEvl_EQUALS;
		std::string locked = "00000000";
		int paramIndexRedirect = INT_MAX;
		int extraCondition = extraConditionTypes::exCon_NULL;
		std::vector<movesetPatchModAction> actions;
	};
	struct movesetPatch
	{
		std::string name = "";
		std::string sourceFile = "";
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
		bool init(std::string filePathIn, std::ostream& logStream);
		std::string fetchString(std::size_t strAddr);
		void summarizeTable(std::size_t tableAddr, std::size_t entryCount, std::size_t offsetShiftSize, std::string prefix = "\t\t", std::ostream& output = std::cout);
		void summarizeOffsetSection(std::ostream& output = std::cout, std::size_t adjustment = 0x80);
		void summarizeAttributeSection(std::ostream& output = std::cout);

		std::vector<std::size_t> changeMatchingParameter(std::vector<std::pair<std::string, std::vector<char>>> functions, std::vector<std::pair<std::string, std::string>> matches, std::size_t parameterOffsetInBytes);

		movesetPatchResult applyMovesetPatch(const movesetPatch& patchIn, std::ostream& logStream);
	};

	struct paramTarget
	{
	private:
		movesetFile* parentPtr = nullptr;

		int targetParamIndex = 0;
		std::size_t targetParamIndexOffset = SIZE_MAX;

		std::vector<char> paramOffset = { CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX };
		std::size_t paramOffsetNum = SIZE_MAX;

		std::vector<char> paramTypeIdentifier = { CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX };
		std::size_t paramTypeIdentifierNum = SIZE_MAX;

		std::vector<char> paramValue = { CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX };
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

#endif
