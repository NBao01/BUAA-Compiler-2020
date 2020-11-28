#include "ir.h"
#include <sstream>
#include <fstream>
#include <cassert>
#include <stack>
#include "table.h"

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

void IrItem::setRes(std::string* newRes) {
	res = newRes;
}

std::string* IrGenerator::tempIdentifierGen(bool rollback) {
	static int i = 0;
	if (rollback) {
		i--;
		return nullptr;
	}
	std::string prefix = "$temp_";
	std::string suffix;
	std::stringstream ss;
	ss << i++;
	ss >> suffix;
	return new std::string(prefix + suffix);
}

std::string* IrGenerator::ifLabelGen() {
	static int i = 0;
	std::string prefix = "If_Else_";
	std::string suffix;
	std::stringstream ss;
	ss << i++;
	ss >> suffix;
	return new std::string(prefix + suffix);
}

std::string* IrGenerator::endifLabelGen() {
	static int i = 0;
	std::string prefix = "Endif_";
	std::string suffix;
	std::stringstream ss;
	ss << i++;
	ss >> suffix;
	return new std::string(prefix + suffix);
}

std::string* IrGenerator::whileLabelGen() {
	static int i = 0;
	std::string prefix = "While_";
	std::string suffix;
	std::stringstream ss;
	ss << i++;
	ss >> suffix;
	return new std::string(prefix + suffix);
}

std::string* IrGenerator::endwhileLabelGen() {
	static int i = 0;
	std::string prefix = "Endwhile_";
	std::string suffix;
	std::stringstream ss;
	ss << i++;
	ss >> suffix;
	return new std::string(prefix + suffix);
}

std::string* IrGenerator::forLabelGen() {
	static int i = 0;
	std::string prefix = "For_";
	std::string suffix;
	std::stringstream ss;
	ss << i++;
	ss >> suffix;
	return new std::string(prefix + suffix);
}

std::string* IrGenerator::endforLabelGen() {
	static int i = 0;
	std::string prefix = "Endfor_";
	std::string suffix;
	std::stringstream ss;
	ss << i++;
	ss >> suffix;
	return new std::string(prefix + suffix);
}

// This is a logic-complex function. Should be very careful, always think about it.
std::string* IrGenerator::switchLabelGen(bool _case, bool _default, bool endswitch, bool lastLabel) {
	static std::stack<int> ijStack;
	static int level = 0;	// switch可能會嵌套，因此記錄當前嵌套層數，否則i、j可能會"一去不復返"
	static int i = -1;
	static int j = 0;
	static int maxI = -1;	// 記錄曾經出現過的最大的i，確保生成的switch標簽不會重複

	if (endswitch) {
		std::string prefix = "Endswitch_";
		std::string suffix;
		std::stringstream ss;
		ss << i;
		ss >> suffix;
		if (lastLabel) {
			level--;
			if (level > 0) {	// 回復外層switch的i和j
				j = ijStack.top();
				ijStack.pop();
				i = ijStack.top();
				ijStack.pop();
			}
		}
		return new std::string(prefix + suffix);
	}
	if (_default) {
		std::string prefix = "Switch_";
		std::string suffix;
		std::stringstream ss;
		ss << i << "_default";
		ss >> suffix;
		return new std::string(prefix + suffix);
	}
	if (_case) {
		std::string prefix = "Switch_";
		std::string suffix;
		std::stringstream ss;
		ss << i << "_case_" << j++;
		ss >> suffix;
		return new std::string(prefix + suffix);
	}
	if (level > 0) {	//	当前是嵌套在switch中的switch语句
		ijStack.push(i);
		ijStack.push(j);
	}
	level++;
	j = 0;
	std::string prefix = "Switch_";
	std::string suffix;
	std::stringstream ss;
	ss << ++maxI;
	i = maxI;
	ss >> suffix;
	return new std::string(prefix + suffix);
}

void IrGenerator::output() {
	std::ofstream out("ir.txt", std::ios_base::out);
	std::vector<IrItem*>::iterator it;
	for (it = IrList.begin(); it != IrList.end(); ++it) {
		int op = (*it)->getOp();
		switch (op) {
		case IR_PRINT:
			out << irInstructions[op] << " ";
			if ((*it)->getLopType() == STRTYPE) {
				out << "\"" << *(*it)->getLop() << "\"" << std::endl;
			}
			else if ((*it)->getLopType() == IDTYPE || (*it)->getLopType() == TMPTYPE || (*it)->getLopType() == TMPTYPE_CH) {
				out << *(*it)->getLop() << std::endl;
			}
			else if ((*it)->getLopType() == INTTYPE) {
				out << (*it)->getLopInt() << std::endl;
			}
			else if ((*it)->getLopType() == CHTYPE) {
				out << "'" << *(*it)->getLop() << "'" << std::endl;
			}
			break;
		case IR_SCAN:
			out << irInstructions[op] << " " << *(*it)->getLop() << std::endl;
			break;
		case IR_ASSIGN:
			out << irInstructions[op] << " ";
			if ((*it)->getLopType() == INTTYPE) {
				out << (*it)->getLopInt();
			}
			else if ((*it)->getLopType() == CHTYPE) {
				out << "'" << *(*it)->getLop() << "'";
			}
			else if ((*it)->getLopType() == IDTYPE || (*it)->getLopType() == TMPTYPE || (*it)->getLopType() == TMPTYPE_CH) {
				out << *(*it)->getLop();
			}
			out << " " << *(*it)->getRes() << std::endl;
			break;
		case IR_ADD:
		case IR_SUB:
		case IR_MUL:
		case IR_DIV:
			out << irInstructions[op] << " ";
			if ((*it)->getLopType() == INTTYPE) {
				out << (*it)->getLopInt();
			}
			else if ((*it)->getLopType() == CHTYPE) {
				out << "'" << *(*it)->getLop() << "'";
			}
			else if ((*it)->getLopType() == IDTYPE || (*it)->getLopType() == TMPTYPE || (*it)->getLopType() == TMPTYPE_CH) {
				out << *(*it)->getLop();
			}
			out << " ";
			if ((*it)->getRopType() == INTTYPE) {
				out << (*it)->getRopInt();
			}
			else if ((*it)->getRopType() == CHTYPE) {
				out << "'" << *(*it)->getRop() << "'";
			}
			else if ((*it)->getRopType() == IDTYPE || (*it)->getRopType() == TMPTYPE || (*it)->getRopType() == TMPTYPE_CH) {
				out << *(*it)->getRop();
			}
			out << " " << *(*it)->getRes() << std::endl;
			break;
		case IR_FUNCDEF:
			out << std::endl;
			out << *(*it)->getLop() << "()" << std::endl;
			break;
		case IR_RET:
		case IR_PUSH:
			out << irInstructions[op] << " ";
			if ((*it)->getLopType() == IDTYPE || (*it)->getLopType() == TMPTYPE || (*it)->getLopType() == TMPTYPE_CH) {
				out << *(*it)->getLop();
			}
			else if ((*it)->getLopType() == CHTYPE) {
				out  << "'" << *(*it)->getLop() << "'";
			}
			else if ((*it)->getLopType() == INTTYPE) {
				out  << (*it)->getLopInt();
			}
			out << std::endl;
			break;
		case IR_PRECALL:
		case IR_CALL:
			out << irInstructions[op] << " " << *(*it)->getLop() << std::endl;
			break;
		case IR_ARRAYGET:
			assert((*it)->getLopType() == IDTYPE);
			out << irInstructions[op] << " " << *(*it)->getLop() << "[";
			assert((*it)->getRopType() != CHTYPE);
			if ((*it)->getRopType() == INTTYPE) {
				out << (*it)->getRopInt();
			}
			else if ((*it)->getRopType() == IDTYPE || (*it)->getRopType() == TMPTYPE || (*it)->getRopType() == TMPTYPE_CH) {
				out << *(*it)->getRop();
			}
			out << "] " << *(*it)->getRes() << std::endl;
			break;
		case IR_ARRAYSET:
			out << irInstructions[op] << " ";
			if ((*it)->getLopType() == INTTYPE) {
				out << (*it)->getLopInt();
			}
			else if ((*it)->getLopType() == CHTYPE) {
				out << "'" << *(*it)->getLop() << "'";
			}
			else if ((*it)->getLopType() == IDTYPE || (*it)->getLopType() == TMPTYPE || (*it)->getLopType() == TMPTYPE_CH) {
				out << *(*it)->getLop();
			}
			out << " " << *(*it)->getRes() << "[";
			assert((*it)->getRopType() != CHTYPE);
			if ((*it)->getRopType() == INTTYPE) {
				out << (*it)->getRopInt();
			}
			else if ((*it)->getRopType() == IDTYPE || (*it)->getRopType() == TMPTYPE || (*it)->getRopType() == TMPTYPE_CH) {
				out << *(*it)->getRop();
			}
			out << "]" << std::endl;
			break;
		case IR_LSS:
		case IR_LEQ:
		case IR_GRE:
		case IR_GEQ:
		case IR_EQL:
		case IR_NEQ:
			out << irInstructions[op] << " ";
			if ((*it)->getLopType() == INTTYPE) {
				out << (*it)->getLopInt();
			}
			else if ((*it)->getLopType() == CHTYPE) {
				out << "'" << *(*it)->getLop() << "'";
			}
			else if ((*it)->getLopType() == IDTYPE || (*it)->getLopType() == TMPTYPE || (*it)->getLopType() == TMPTYPE_CH) {
				out << *(*it)->getLop();
			}
			out << " ";
			if ((*it)->getRopType() == INTTYPE) {
				out << (*it)->getRopInt();
			}
			else if ((*it)->getRopType() == CHTYPE) {
				out << "'" << *(*it)->getRop() << "'";
			}
			else if ((*it)->getRopType() == IDTYPE || (*it)->getRopType() == TMPTYPE || (*it)->getRopType() == TMPTYPE_CH) {
				out << *(*it)->getRop();
			}
			out << std::endl;
			break;
		case IR_LABEL:
			out << *(*it)->getRes() << ":" << std::endl;
			break;
		case IR_BZ:
		case IR_BNZ:
		case IR_GOTO: 
			out << irInstructions[op] << " " << *(*it)->getRes() << std::endl;
			break;
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
	// If R-value of Assign statement is a expression, we don't need an assign ir, instead of modify last ir's res.
	if ((lopType == TMPTYPE || lopType == TMPTYPE_CH) && lop->find("$RET") == std::string::npos) {
		IrItem* lastIr = IrList.back();
		assert(*lastIr->getRes() == *lop);
		lastIr->setRes(res);
		tempIdentifierGen(true);	// rollback a temp identifier
	}
	else {
		IrList.push_back(new IrItem(IR_ASSIGN, lopType, NOTYPE, lopInt, 0, lop, nullptr, res));
	}
}

std::string* IrGenerator::addNormalIr(int op, int lopType, int ropType, int lopInt, int ropInt,
	std::string* lop, std::string* rop, std::string* res) {
	std::string* ret = nullptr;
	ret = res ? res : tempIdentifierGen();
	IrList.push_back(new IrItem(op, lopType, ropType, lopInt, ropInt, lop, rop, ret));
	return ret;
}

void IrGenerator::addFuncDefIr(int type, std::string* lop) {
	assert(type == IDTYPE);
	IrList.push_back(new IrItem(IR_FUNCDEF, type, NOTYPE, 0, 0, lop, nullptr, nullptr));
}

void IrGenerator::addReturnIr(int type, int num, std::string* lop) {
	IrList.push_back(new IrItem(IR_RET, type, NOTYPE, num, 0, lop, nullptr, nullptr));
}

void IrGenerator::addPrecallIr(int type, std::string* lop) {
	assert(type == IDTYPE);
	IrList.push_back(new IrItem(IR_PRECALL, type, NOTYPE, 0, 0, lop, nullptr, nullptr));
}

void IrGenerator::addCallIr(int type, std::string* lop, std::string* retLabel) {
	assert(type == IDTYPE);
	IrList.push_back(new IrItem(IR_CALL, type, NOTYPE, 0, 0, lop, nullptr, retLabel));
}

void IrGenerator::addPushIr(int type, int num, std::string* lop) {
	IrList.push_back(new IrItem(IR_PUSH, type, NOTYPE, num, 0, lop, nullptr, nullptr));
}

void IrGenerator::addComparisonIr(
	int op, int lopType, int ropType, int lopInt, int ropInt, std::string* lop, std::string* rop) {
	IrList.push_back(new IrItem(op, lopType, ropType, lopInt, ropInt, lop, rop, nullptr));
}

void IrGenerator::addLabelIr(std::string* label) {
	IrList.push_back(new IrItem(IR_LABEL, NOTYPE, NOTYPE, 0, 0, nullptr, nullptr, label));
}

void IrGenerator::addBnzIr(std::string* label) {
	IrList.push_back(new IrItem(IR_BNZ, NOTYPE, NOTYPE, 0, 0, nullptr, nullptr, label));
}

void IrGenerator::addBzIr(std::string* label) {
	IrList.push_back(new IrItem(IR_BZ, NOTYPE, NOTYPE, 0, 0, nullptr, nullptr, label));
}

void IrGenerator::addGotoIr(std::string* label) {
	IrList.push_back(new IrItem(IR_GOTO, NOTYPE, NOTYPE, 0, 0, nullptr, nullptr, label));
}

// Find the first label after the Switch_x label, insert vector before to the case0_or_default label's before.
void IrGenerator::addToLastSwitch(std::vector<IrItem*>* before, std::string* label_switch) {
	std::vector<IrItem*>::iterator it, case0_or_default_it;
	for (it = IrList.end() - 1; ; --it) {
		if ((*it)->getOp() == IR_LABEL && *label_switch == *(*it)->getRes()) {
			break;
		}
		if ((*it)->getOp() == IR_LABEL) {
			case0_or_default_it = it;
		}
	}

	IrList.insert(case0_or_default_it, before->begin(), before->end());
}

IrItem* IrGenerator::lastIr() {
	return IrList.back();
}