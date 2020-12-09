#ifndef __REGFILEMANAGER_H__
#define __REGFILEMANAGER_H__

#include <string>
#include "table.h"
#include "mipsDefinitions.h"

class Reg {
private:
	int id;
	std::string* label;
	bool valid;
	bool dirty;
	bool temp;
	bool inStack;	// For env save and restore
	int nOfArray;
public:
	Reg(int id);
	int getId();
	std::string* getLabel();
	bool isValid();
	bool isDirty();
	bool isTemp();
	bool isInStack();
	int getNOfArray();
	void setLabel(std::string* label);
	void setValid(bool valid);
	void setDirty(bool dirty);
	void setTemp(bool temp);
	void setInStack(bool inStack);
	void setNOfArray(int n);
};

extern Reg* regfile[32];

class RegfileManager {
private:
	static Reg* getTempReg();
	static Reg* getSavedReg();
public:
	static void init();
	static Reg* mapping(TableItem* ti, bool load = false, int specificReg = 0, int sp = $sp);
	static Reg* mappingTemp(std::string* label);
	static Reg* mappingTemp();
	static int searchTemp(std::string* label);
	static void writeAllBack();
	static void writeSBack();
	static void writeABack();
	static void flush();
	static void saveEnv();
	static void restoreEnv();
	static void setInvalid(int regId);
};

#endif // !__REGFILEMANAGER_H__
