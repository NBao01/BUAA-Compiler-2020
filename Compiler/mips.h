#ifndef __MIPS_H__
#define __MIPS_H__

#include <string>
#include <vector>
#include "mipsDefinitions.h"

class DataItem {
private:
	std::string* label;
	std::string* prototype;
	int dataType;
	int data;
	std::string* strData;
	int cache;
public:
	DataItem(std::string* label, std::string* prototype, int dataType, int data, std::string* strData, int cache);
	std::string* getLabel();
	std::string* getPrototype();
	int getDataType();
	int getData();
	std::string* getStrData();
};

class TextItem {
private:
	int instr;
	int rs;
	int rt;
	int rd;
	int immediate;
	std::string* label;
public:
	TextItem(int instr, int rs, int rt, int rd, int immediate, std::string* label);
	int getInstr();
	int getRs();
	int getRt();
	int getRd();
	int getImmediate();
	std::string* getLabel();
};

extern std::vector<DataItem*> dataSegment;
extern std::vector<TextItem*> textSegment;

class MipsGenerator {
private:
	static std::string* strConLabelGen();
public:
	static void output();
	static void generate();
	static void addData(std::string* label, std::string* strData);
	static void addData(std::string* label, std::string* prototype, int data);
	static void addR(int instr, int rs, int rt, int rd);
	static void addI(int instr, int rs, int rt, int immediate, std::string* label);
	static void addSyscall();
};

#endif // !__MIPS_H__
