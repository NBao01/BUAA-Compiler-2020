#ifndef __TABLE_H__
#define __TABLE_H__

#include <string>
#include <vector>
#include "abstractSyntaxTree.h"

// definition for type
#define CONST	0
#define VAR		1
#define FUNC	2
#define PARAM	3

// definition for retType
#define VOID	0
#define INT		1
#define CHAR	2

class TableItem {
private:
	std::string* name;
	int type;		// CONST, VAR, FUNC, PARAM
	int retType;	// VOID, INT, CHAR
	int scope;
	int dimension;
	int paramNum;
	std::vector<int>* paramsRetType;
public:
	static int scope_i;
	TableItem(std::string* name, int type, int retType, int scope);
	static TableItem* newConstTableItem(std::string* name, int type, int retType);
	static TableItem* newVarTableItem(std::string* name, int type, int retType, int dimension);
	static TableItem* newFuncTableItem(std::string* name, int type, int retType);
	static TableItem* newParamTableItem(std::string* name, int type, int retType);
	void setDimension(int dimension);
	void setParams(int paramNum, std::vector<int>* paramsRetType);
	bool isSameScope(int curScope);
	bool isSameName(std::string* name);
	int getType();
	int getRetType();
	int getParamNum();
	std::vector<int>* getParamsRetType();
};

extern std::vector<TableItem*> table;

class TableTools {
private:
	static std::string* name;
	static int type;
	static int retType;
	static int dimension;
	static int it_prev; // iterator of wordlist, stored to ensure the range of parts of the program.
public:
	// Table Construct Tools
	static void addConsts(int it);
	static void addVars(int it);
	static void addFunc(int it);
	// Table Search Tools
	static bool isVoidFunc(std::string* word);
	static bool isCharType(SymbolNode* node);
	static bool errorJudgerC(Word* word);
	static bool errorJudgerD(Word* word, SymbolNode* node);
	static bool errorJudgerE(Word* word, SymbolNode* node);
	static bool errorJudgerGH(int stage, int key, int line = 0);
	static bool errorJudgerJ(Word* word);
	static bool errorJudgerO(SymbolNode* node, int stage);
};

#endif // !__TABLE_H__
