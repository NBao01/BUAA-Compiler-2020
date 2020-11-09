#include "mips.h"
#include "ir.h"
#include <sstream>
#include <fstream>
#include <cctype>
#include "table.h"
#include "regfileManager.h"

std::vector<DataItem*> dataSegment;
std::vector<TextItem*> textSegment;

DataItem::DataItem(std::string* label, std::string* prototype, int dataType, int data, std::string* strData, int cache) {
	this->label = label;
	this->prototype = prototype;
	this->dataType = dataType;
	this->data = data;
	this->strData = strData;
	this->cache = cache;
}

std::string* DataItem::getLabel() {
	return label;
}

std::string* DataItem::getPrototype() {
	return prototype;
}

int DataItem::getDataType() {
	return dataType;
}

int DataItem::getData() {
	return data;
}

std::string* DataItem::getStrData() {
	return strData;
}

TextItem::TextItem(int instr, int rs, int rt, int rd, int immediate, std::string* label) {
	this->instr = instr;
	this->rs = rs;
	this->rt = rt;
	this->rd = rd;
	this->immediate = immediate;
	this->label = label;
}

int TextItem::getInstr() {
	return instr;
}

int TextItem::getRs() {
	return rs;
}

int TextItem::getRt() {
	return rt;
}

int TextItem::getRd() {
	return rd;
}

int TextItem::getImmediate() {
	return immediate;
}

std::string* TextItem::getLabel() {
	return label;
}

std::string* tolower(std::string* str) {
	if (str == nullptr) {
		return nullptr;
	}
	std::string* ret = new std::string();
	for (int i = 0; i < str->size(); i++) {
		*ret += tolower(str->at(i));
	}
	return ret;
}

std::string* MipsGenerator::strConLabelGen() {
	static int i = 0;
	std::string prefix = "strcon_";
	std::string suffix;
	std::stringstream ss;
	ss << i++;
	ss >> suffix;
	return new std::string(prefix + suffix);
}

void MipsGenerator::addData(std::string* label, std::string* strData) {
	dataSegment.push_back(new DataItem(tolower(label), nullptr,  _ASCIIZ, 0, strData, 0));
}

void MipsGenerator::addData(std::string* label, std::string* prototype, int data) {
	dataSegment.push_back(new DataItem(tolower(label), tolower(prototype), _WORD, data, nullptr, 0));
}

void MipsGenerator::addR(int instr, int rs, int rt, int rd) {
	textSegment.push_back(new TextItem(instr, rs, rt, rd, 0, nullptr));
}

void MipsGenerator::addI(int instr, int rs, int rt, int immediate, std::string* label) {
	textSegment.push_back(new TextItem(instr, rs, rt, 0, immediate, tolower(label)));
}

void MipsGenerator::addSyscall() {
	textSegment.push_back(new TextItem(MIPS_SYSCALL, 0, 0, 0, 0, nullptr));
}

void MipsGenerator::generate() {
	RegfileManager::init();
	addData(new std::string("endl"), new std::string("\\n"));
	std::vector<IrItem*>::iterator it;
	for (it = IrList.begin(); it != IrList.end(); it++) {
		IrItem* ir = *it;
		if (ir->getOp() == IR_PRINT) {
			if (ir->getLopType() == STRTYPE) {
				std::string* strLabel = strConLabelGen();
				addData(strLabel, ir->getLop());
				addI(MIPS_LA, 0, $a0, 0, strLabel);
				addI(MIPS_LI, 0, $v0, 4, nullptr);
				addSyscall();
			}
			else if (ir->getLopType() == IDTYPE) {
				std::string* label; int type;
				TableTools::search(tolower(ir->getLop()), &type, &label);
				addI(MIPS_LW, 0, $a0, 0, label);
				if (type == INT) {
					addI(MIPS_LI, 0, $v0, 1, nullptr);
				}
				else if (type == CHAR) {
					addI(MIPS_LI, 0, $v0, 11, nullptr);
				}
				addSyscall();
				addI(MIPS_LA, 0, $a0, 0, new std::string("endl"));
				addI(MIPS_LI, 0, $v0, 4, nullptr);
				addSyscall();
			}
			else if (ir->getLopType() == TMPTYPE) {
				int rs = RegfileManager::searchTemp(ir->getLop());
				addR(MIPS_ADD, rs, $zero, $a0);
				addI(MIPS_LI, 0, $v0, 1, nullptr);
				addSyscall();
				addI(MIPS_LA, 0, $a0, 0, new std::string("endl"));
				addI(MIPS_LI, 0, $v0, 4, nullptr);
				addSyscall();
			}
			else if (ir->getLopType() == INTTYPE) {
				addI(MIPS_LI, 0, $a0, ir->getLopInt(), nullptr);
				addI(MIPS_LI, 0, $v0, 1, nullptr);
				addSyscall();
				addI(MIPS_LA, 0, $a0, 0, new std::string("endl"));
				addI(MIPS_LI, 0, $v0, 4, nullptr);
				addSyscall();
			}
			else if (ir->getLopType() == CHTYPE) {
				addI(MIPS_LI, 0, $a0, (int)ir->getLop()->at(0), nullptr);
				addI(MIPS_LI, 0, $v0, 11, nullptr);
				addSyscall();
				addI(MIPS_LA, 0, $a0, 0, new std::string("endl"));
				addI(MIPS_LI, 0, $v0, 4, nullptr);
				addSyscall();
			}
		}
		else if (ir->getOp() == IR_SCAN) {
			std::string* label; int type;
			TableTools::search(tolower(ir->getLop()), &type, &label);
			if (type == INT) {
				addI(MIPS_LI, 0, $v0, 5, nullptr);
			}
			else if (type == CHAR) {
				addI(MIPS_LI, 0, $v0, 12, nullptr);
			}
			addSyscall();
			addI(MIPS_SW, 0, $v0, 0, label);
		}
		else if (ir->getOp() == IR_ASSIGN) {
			std::string* label; int type;
			if (ir->getLopType() == INTTYPE) {
				addI(MIPS_LI, 0, $a0, ir->getLopInt(), nullptr);
				TableTools::search(tolower(ir->getRes()), &type, &label);
				addI(MIPS_SW, 0, $a0, 0, label);
			}
			else if (ir->getLopType() == CHTYPE) {
				addI(MIPS_LI, 0, $a0, (int)ir->getLop()->at(0), nullptr);
				TableTools::search(tolower(ir->getRes()), &type, &label);
				addI(MIPS_SW, 0, $a0, 0, label);
			}
			else if (ir->getLopType() == IDTYPE) {
				TableTools::search(tolower(ir->getLop()), &type, &label);
				addI(MIPS_LW, 0, $a0, 0, label);
				TableTools::search(tolower(ir->getRes()), &type, &label);
				addI(MIPS_SW, 0, $a0, 0, label);
			}
			else if (ir->getLopType() == TMPTYPE) {
				int rt = RegfileManager::searchTemp(ir->getLop());
				TableTools::search(tolower(ir->getRes()), &type, &label);
				addI(MIPS_SW, 0, rt, 0, label);
			}
		}
		else if (ir->getOp() >= IR_ADD && ir->getOp() <= IR_DIV) {
			int rs = 0, rt = 0;
			std::string* label; int type;
			if (ir->getLopType() == INTTYPE) {
				Reg* r = RegfileManager::getTempReg();
				addI(MIPS_LI, 0, rs = r->getId(), ir->getLopInt(), nullptr);
			}
			else if (ir->getLopType() == CHTYPE) {
				Reg* r = RegfileManager::getTempReg();
				addI(MIPS_LI, 0, rs = r->getId(), (int)ir->getLop()->at(0), nullptr);
			}
			else if (ir->getLopType() == IDTYPE) {
				TableTools::search(tolower(ir->getLop()), &type, &label);
				Reg* r = RegfileManager::getTempReg();
				addI(MIPS_LW, 0, rs = r->getId(), 0, label);
				r->setLabel(label);
			}
			else if (ir->getLopType() == TMPTYPE) {
				rs = RegfileManager::searchTemp(ir->getLop());
			}

			if (ir->getRopType() == INTTYPE) {
				Reg* r = RegfileManager::getTempReg();
				addI(MIPS_LI, 0, rt = r->getId(), ir->getRopInt(), nullptr);
			}
			else if (ir->getRopType() == CHTYPE) {
				Reg* r = RegfileManager::getTempReg();
				addI(MIPS_LI, 0, rt = r->getId(), (int)ir->getRop()->at(0), nullptr);
			}
			else if (ir->getRopType() == IDTYPE) {
				TableTools::search(tolower(ir->getRop()), &type, &label);
				Reg* r = RegfileManager::getTempReg();
				addI(MIPS_LW, 0, rt = r->getId(), 0, label);
				r->setLabel(label);
			}
			else if (ir->getRopType() == TMPTYPE) {
				rt = RegfileManager::searchTemp(ir->getRop());
			}

			Reg* r = RegfileManager::getTempReg();
			addR(ir->getOp(), rs, rt, r->getId());
			r->setLabel(tolower(ir->getRes()));
		}
	}
}

void MipsGenerator::output() {
	std::ofstream out("mips.txt", std::ios_base::out);

	out << ".data" << std::endl;
	std::vector<DataItem*>::iterator dataIt;
	for (dataIt = dataSegment.begin(); dataIt != dataSegment.end(); ++dataIt) {
		if ((*dataIt)->getDataType() == _ASCIIZ) {
			out << "\t" << *(*dataIt)->getLabel() << ": .asciiz " << 
				"\"" << *(*dataIt)->getStrData() << "\"" << std::endl;
		}
		else if ((*dataIt)->getDataType() == _WORD) {
			out << "\t" << *(*dataIt)->getLabel() << ": .word " << (*dataIt)->getData() << std::endl;
		}
	}

	out << ".text" << std::endl;
	std::vector<TextItem*>::iterator textIt;
	for (textIt = textSegment.begin(); textIt != textSegment.end(); ++textIt) {
		if ((*textIt)->getInstr() == MIPS_LI) {
			out << "\tli " << regstr[(*textIt)->getRt()] << ", " << (*textIt)->getImmediate() << std::endl;
		}
		else if ((*textIt)->getInstr() == MIPS_LA) {
			out << "\tla " << regstr[(*textIt)->getRt()] << ", " << *(*textIt)->getLabel() << std::endl;
		}
		else if ((*textIt)->getInstr() == MIPS_SYSCALL) {
			out << "\tsyscall" << std::endl;
		}
		else if ((*textIt)->getInstr() == MIPS_SW) {
			out << "\tsw " << regstr[(*textIt)->getRt()] << ", " << *(*textIt)->getLabel() << std::endl;
		}
		else if ((*textIt)->getInstr() == MIPS_LW) {
			out << "\tlw " << regstr[(*textIt)->getRt()] << ", " << *(*textIt)->getLabel() << std::endl;
		}
		else if ((*textIt)->getInstr() >= MIPS_ADD && (*textIt)->getInstr() <= MIPS_DIV) {
			out << "\t" << mipsInstructions[(*textIt)->getInstr()] << " " <<
				regstr[(*textIt)->getRd()] << ", " <<
				regstr[(*textIt)->getRs()] << ", " <<
				regstr[(*textIt)->getRt()] << std::endl;
		}
	}
}