#ifndef __REGFILEMANAGER_H__
#define __REGFILEMANAGER_H__

#include <string>
#include "table.h"

class Reg {
private:
	int id;
	std::string* label;
	bool valid;
	bool dirty;
	bool temp;
public:
	Reg(int id);
	int getId();
	std::string* getLabel();
	bool isValid();
	bool isDirty();
	bool isTemp();
	void setLabel(std::string* label);
	void setValid(bool valid);
	void setDirty(bool dirty);
	void setTemp(bool temp);
};

extern Reg* regfile[32];

class RegfileManager {
private:
	static Reg* getTempReg();
	static Reg* getSavedReg();
public:
	static void init();
	static Reg* mapping(TableItem* ti, bool load = false, int specificReg = 0);
	static Reg* mappingTemp(std::string* label);
	static Reg* mappingTemp();
	static int searchTemp(std::string* label);
	static void writeAllBack();
	static void flush();
	static void saveEnv();
	static void restoreEnv();
};

#endif // !__REGFILEMANAGER_H__
