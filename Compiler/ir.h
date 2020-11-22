#ifndef __IR_H__
#define __IR_H__

#include <string>
#include <vector>
#include "irDefinitions.h"

// lop type and rop type
#define NOTYPE		0	// no type, lop/rop is nullptr
#define IDTYPE		1	// identifier type, lop/rop is the identifier
#define INTTYPE		2	// int type, lopInt/ropInt is the integer
#define CHTYPE		3	// char type, lop/rop is the char
#define STRTYPE		4	// string type, lop/rop is the string
#define TMPTYPE		5	// temp type, lop/rop is the temp identifier

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
	static void addFuncDefIr(int type, std::string* lop);
	static void addReturnIr(int type, int num, std::string* lop);
	static void addCallIr(int type, std::string* lop);
	static void addPushIr(int type, int num, std::string* lop);
};

#endif // !__IR_H__
