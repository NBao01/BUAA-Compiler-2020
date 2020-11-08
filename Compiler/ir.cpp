#include "ir.h"
#include <sstream>
#include <fstream>

std::vector<IrItem*> IrList;

IrItem::IrItem(int op, int lopType, int ropType, int lopInt, int ropInt, 
	std::string* lop, std::string* rop, std::string* res) {
	this->op = op;
	this->lopType = lopType;
	this->ropType = ropType;
	this->lopInt = lopInt;
	this->ropInt = ropInt;
	this->lop = lop;
	this->rop = rop;
	this->res = res;
}

IrItem::IrItem(int op, int lopType, std::string* lop) {
	this->op = op;
	this->lopType = lopType;
	this->ropType = NOTYPE;
	this->lopInt = 0;
	this->ropInt = 0;
	this->lop = lop;
	this->rop = nullptr;
	this->res = nullptr;
}

int IrItem::getOp() {
	return op;
}

int IrItem::getLopType() {
	return lopType;
}

int IrItem::getRopType() {
	return ropType;
}

int IrItem::getLopInt() {
	return lopInt;
}

int IrItem::getRopInt() {
	return ropInt;
}

std::string* IrItem::getLop() {
	return lop;
}

std::string* IrItem::getRop() {
	return rop;
}

std::string* IrItem::getRes() {
	return res;
}

std::string* IrGenerator::labelGen() {
	static int i = 0;
	std::string prefix = "temp_";
	std::string suffix;
	std::stringstream ss;
	ss << i++;
	ss >> suffix;
	return new std::string(prefix + suffix);
}

void IrGenerator::output() {
	std::ofstream out("ir.txt", std::ios_base::out);
	std::vector<IrItem*>::iterator it;
	for (it = IrList.begin(); it != IrList.end(); ++it) {
		int op = (*it)->getOp();
		if (op == IR_PRINT) {
			if ((*it)->getLopType() == STRTYPE) {
				out << irInstructions[op] << " " << "\"" << *(*it)->getLop() << "\"" << std::endl;
			}
			else if ((*it)->getLopType() == IDTYPE) {
				out << irInstructions[op] << " " << *(*it)->getLop() << std::endl;
			}
		}
		else if (op == IR_SCAN) {
			out << irInstructions[op] << " " << *(*it)->getLop() << std::endl;
		}
		else if (op == IR_ASSIGN) {
			out << irInstructions[op] << " ";
			if ((*it)->getLopType() == INTTYPE) {
				out << (*it)->getLopInt();
			}
			else if ((*it)->getLopType() == CHTYPE) {
				out << "'" << *(*it)->getLop() << "'";
			}
			else {
				out << *(*it)->getLop();
			}
			out << " " << *(*it)->getRes() << std::endl;
		}
		else if (op >= IR_ADD && op <= IR_DIV) {
			out << irInstructions[op] << " ";
			if ((*it)->getLopType() == INTTYPE) {
				out << (*it)->getLopInt();
			}
			else if ((*it)->getLopType() == CHTYPE) {
				out << "'" << *(*it)->getLop() << "'";
			}
			else {
				out << *(*it)->getLop();
			}
			out << " ";
			if ((*it)->getRopType() == INTTYPE) {
				out << (*it)->getRopInt();
			}
			else if ((*it)->getRopType() == CHTYPE) {
				out << "'" << *(*it)->getRop() << "'";
			}
			else {
				out << *(*it)->getRop();
			}
			out << " " << *(*it)->getRes() << std::endl;
		}
	}
	out.close();
}

void IrGenerator::addPrintStrIr(std::string* str) {
	IrList.push_back(new IrItem(IR_PRINT, STRTYPE, str));
}

void IrGenerator::addPrintExpIr(int type, int num, std::string* str) {
	IrList.push_back(new IrItem(IR_PRINT, type, NOTYPE, num, 0, str, nullptr, nullptr));
}

void IrGenerator::addScanIr(std::string* str) {
	IrList.push_back(new IrItem(IR_SCAN, IDTYPE, str));
}

void IrGenerator::addAssignIr(std::string* res, int lopType, int lopInt, std::string* lop) {
	IrList.push_back(new IrItem(IR_ASSIGN, lopType, NOTYPE, lopInt, 0, lop, nullptr, res));
}

std::string* IrGenerator::addNormalIr(int op, int lopType, int ropType, int lopInt, int ropInt,
	std::string* lop, std::string* rop, std::string* res) {
	std::string* ret = res ? res : labelGen();
	IrList.push_back(new IrItem(op, lopType, ropType, lopInt, ropInt, lop, rop, ret));
	return ret;
}