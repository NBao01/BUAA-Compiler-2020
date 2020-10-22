#ifndef __TABLE_H__
#define __TABLE_H__

#include <string>
#include <vector>
#include "abstractSyntaxTree.h"

// definition for type
#define CONST	0
#define VAR		1
#define FUNC	2

// definition for retType
#define VOID	0
#define INT		1
#define CHAR	2

class TableItem {
private:
	std::string* name;
	int type;
	int retType;
	int scope;
	int dimension;
	int paramNum;
	std::vector<int>* params;
	static int scope_i;
public:
	TableItem(std::string* name, int type, int retType);
	static TableItem* newConstTableItem(std::string* name, int type, int retType);
	static TableItem* newVarTableItem(std::string* name, int type, int retType, int dimension);
	static TableItem* newFuncTableItem(std::string* name, int type, int retType, int paramNum, std::vector<int>* params);
	void setDimension(int dimension);
	void setParams(int paramNum, std::vector<int>* params);
};

extern std::vector<TableItem*> table;

class TableTools {
private:
	static std::string* name;
	static int type;
	static int retType;
	static void addConstsDfs(SymbolNode* node);
public:
	static void addConsts(SymbolNode* node);
	//void setName(std::string* name);
	//void setType(int type);
	//void setRetType(int retType);
};

#endif // !__TABLE_H__
