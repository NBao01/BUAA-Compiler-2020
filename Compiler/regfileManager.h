#ifndef __REGFILEMANAGER_H__
#define __REGFILEMANAGER_H__

#include <string>

class Reg {
private:
	int id;
	std::string* label;
	bool valid;
	bool dirty;
public:
	Reg(int id);
	int getId();
	std::string* getLabel();
	bool getValid();
	bool getDirty();
	void setLabel(std::string* label);
	void setValid(bool valid);
	void setDirty(bool dirty);
};

extern Reg* regfile[32];

class RegfileManager {
public:
	static void init();
	static Reg* getTempReg();
	static Reg* getSavedReg();
	static int searchTemp(std::string* label);
};

#endif // !__REGFILEMANAGER_H__
