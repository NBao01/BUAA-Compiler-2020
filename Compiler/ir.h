#ifndef __IR_H__
#define __IR_H__

#include <string>
#include <vector>
#include "irDefinitions.h"

// lop type and rop type
#define NOTYPE		0
#define IDTYPE		1
#define INTTYPE		2
#define CHTYPE		3
#define STRTYPE		4
#define TMPTYPE		5

class IrItem {
private:
	int op;
	int lopType;
	int ropType;
	int lopInt;
	int ropInt;
	std::string* lop;
	std::string* rop;
	std::string* res;
public:
	IrItem(int op, int lopType, int ropType, int lopInt, int ropInt, 
		std::string* lop, std::string* rop, std::string* res);
	IrItem(int op, int lopType, std::string* lop);
	int getOp();
	int getLopType();
	int getRopType();
	int getLopInt();
	int getRopInt();
	std::string* getLop();
	std::string* getRop();
	std::string* getRes();
};

extern std::vector<IrItem*> IrList;

class IrGenerator {
private:
	static std::string* labelGen();
public:
	static void output();
	static void addPrintStrIr(std::string* str); // add 'printf(string)' IR
	static void addPrintExpIr(int type, int num, std::string* str); // add 'printf(expression)' IR
	static void addScanIr(std::string* str);     // add 'scanf(identifier)' IR
	static void addAssignIr(std::string* res, int lopType, int lopInt, std::string* lop);
	static std::string* addNormalIr( // add calculation IR, return result identifier(temp or permanent)
		int op, int lopType, int ropType, int lopInt, int ropInt,
		std::string* lop, std::string* rop, std::string* res = nullptr);
};

#endif // !__IR_H__
