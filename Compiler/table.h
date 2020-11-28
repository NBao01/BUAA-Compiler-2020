#ifndef __TABLE_H__
#define __TABLE_H__

#include <string>
#include <vector>
#include "abstractSyntaxTree.h"

class Reg;

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
	int dims[2];
	int paramNum;
	std::vector<int>* paramsRetType;
	std::string* label;
	int offset;
	bool hasInitialValue;
	int initialValue;
	std::vector<int>* initialValues;
	int scopeInside;	// For function, store their inside scope
	Reg* cache;
public:
	static int scope_i;
	static int offset_i;
	TableItem(std::string* name, int type, int retType, int scope);
	static TableItem* newConstTableItem(std::string* name, int type, int retType, int initialValue);
	static TableItem* newVarTableItem(std::string* name, int type, int retType,
		int dimension,int dim0, int dim1, bool hasInitialValue, int initialValue, std::vector<int>* initialValues);
	static TableItem* newFuncTableItem(std::string* name, int type, int retType);
	static TableItem* newParamTableItem(std::string* name, int type, int retType);
	bool isSameScope(int curScope);
	bool isSameName(std::string* name);
	// Getters
	std::string* getName();
	int getType();
	int getRetType();
	int getScope();
	int getDimension();
	int getDim0();
	int getDim1();
	int getParamNum();
	std::vector<int>* getParamsRetType();
	std::string* getLabel();
	int getOffset();
	bool getHasInitialValue();
	int getInitialValue();
	std::vector<int>* getInitialValues();
	int getScopeInside();
	Reg* getCache();
	// Setters
	void setDimension(int dimension, int dim0, int dim1);
	void setParams(int paramNum, std::vector<int>* paramsRetType);
	void setLabel(std::string* label);
	void setOffset(int offset);
	void setHasInitialValue(bool hasInitialValue);
	void setInitialValue(int initialValue);
	void setInitialValues(std::vector<int>* initialValues);
	void setScopeInside(int scopeInside);
	void setCache(Reg* reg);
};

extern std::vector<TableItem*> table;

class TableTools {
private:
	static std::string* name;
	static int type;
	static int retType;
	static int it_prev; // iterator of wordlist, stored to ensure the range of parts of the program.
	static std::vector<int> stackSpace; // stack space of each scope, stackSpace[0] is scope_1's space.
public:
	// Stack Tools
	static void setStackSpaceOfScope(int space);
	static int getstackSpaceOfScope(int scope);
	static int getstackSpaceOfScope(std::string* funcName);
	// Table Construct Tools
	static void addConsts(int it);
	static void addVars(int it);
	static void addFunc(int it);
	static int parseInitialValue(int& i);
	// Table Search Tools
	static bool isVoidFunc(std::string* word);
	static bool isCharType(SymbolNode* node);
	static bool errorJudgerC(Word* word);
	static bool errorJudgerD(Word* word, SymbolNode* node);
	static bool errorJudgerE(Word* word, SymbolNode* node);
	static bool errorJudgerGH(int stage, int key, int line = 0);
	static bool errorJudgerJ(Word* word);
	static bool errorJudgerO(SymbolNode* node, int stage);
	static TableItem* search(std::string* name, int scope);
	static TableItem* searchByLabel(std::string* label);
	// Cache Tool
	static void cacheFlush();
};

#endif // !__TABLE_H__
