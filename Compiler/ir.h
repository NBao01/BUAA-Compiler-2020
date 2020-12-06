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
#define TMPTYPE_CH	6	// temp type, but for char.

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
	void setRes(std::string* newRes);
	void setLopType(int lopType);
	void setRopType(int ropType);
	void setLopInt(int lopInt);
	void setRopInt(int ropInt);
	void setLop(std::string* lop);
	void setRop(std::string* rop);
	void setOp(int op);
};

extern std::vector<IrItem*> IrList;

class IrGenerator {
public:
	static std::string* tempIdentifierGen(bool rollback = false);
	static std::string* ifLabelGen();
	static std::string* endifLabelGen();
	static std::string* whileLabelGen();
	static std::string* endwhileLabelGen();
	static std::string* forLabelGen();
	static std::string* endforLabelGen();
	static std::string* switchLabelGen(bool _case = false, bool _default = false, bool endswitch = false, bool lastLabel = false);
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
	static void addPrecallIr(int type, std::string* lop);
	static void addCallIr(int type, std::string* lop, std::string* retLabel = nullptr);
	static void addPushIr(int type, int num, std::string* lop);
	static void addComparisonIr(
		int op, int lopType, int ropType, int lopInt, int ropInt,std::string* lop, std::string* rop);
	static void addLabelIr(std::string* label);
	static void addBnzIr(std::string* label);	// 满足条件
	static void addBzIr(std::string* label);	// 不满足条件
	static void addGotoIr(std::string* label);
	static void addToLastSwitch(std::vector<IrItem*>* before, std::string* label_switch);
	static IrItem* lastIr();
};

#endif // !__IR_H__
