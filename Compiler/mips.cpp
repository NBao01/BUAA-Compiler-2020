#include "mips.h"
#include <sstream>
#include <fstream>
#include <cctype>
#include "regfileManager.h"
#include <cassert>

std::vector<DataItem*> dataSegment;
std::vector<TextItem*> textSegment;

DataItem::DataItem(std::string* label, std::string* prototype, int dataType, int data, std::string* strData) {
	this->label = label;
	this->prototype = prototype;
	this->dataType = dataType;
	this->data = data;
	this->strData = strData;
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

std::string* MipsGenerator::strConLabelGen() {
	static int i = 0;
	std::string prefix = "strcon_";
	std::string suffix;
	std::stringstream ss;
	ss << i++;
	ss >> suffix;
	return new std::string(prefix + suffix);
}

int MipsGenerator::bInstrJudger(int cmpIr, int bIr) {
	switch (cmpIr) {
	case IR_EQL:
		return bIr == IR_BNZ ? MIPS_BEQ : MIPS_BNE;
	case IR_NEQ:
		return bIr == IR_BNZ ? MIPS_BNE : MIPS_BEQ;
	case IR_LSS:
		return bIr == IR_BNZ ? MIPS_BLT : MIPS_BGE;
	case IR_LEQ:
		return bIr == IR_BNZ ? MIPS_BLE : MIPS_BGT;
	case IR_GRE:
		return bIr == IR_BNZ ? MIPS_BGT : MIPS_BLE;
	case IR_GEQ:
		return bIr == IR_BNZ ? MIPS_BGE : MIPS_BLT;
	}
	return 0;
}

int MipsGenerator::getRegL0R1(IrItem* ir, int curScope, int lr) {
	int regId = 0;
	Reg* r = nullptr;
	TableItem* ti = nullptr;
	if (lr == 0) {		// Lop
		if (ir->getLopType() == INTTYPE) {
			r = RegfileManager::mappingTemp();
			addI(MIPS_LI, 0, regId = r->getId(), ir->getLopInt(), nullptr);
		}
		else if (ir->getLopType() == CHTYPE) {
			r = RegfileManager::mappingTemp();
			addI(MIPS_LI, 0, regId = r->getId(), (int)ir->getLop()->at(0), nullptr);
		}
		else if (ir->getLopType() == IDTYPE) {
			ti = TableTools::search(ir->getLop(), curScope);
			// in RegfileManager::mapping(ti, true), load = true, intend read.
			r = ti->getCache() == nullptr ? RegfileManager::mapping(ti, true) : ti->getCache();
			regId = r->getId();
		}
		else if (ir->getLopType() == TMPTYPE || ir->getLopType() == TMPTYPE_CH) {
			//regId = (ir->getLop())->find("$RET") != std::string::npos ? $v1 : RegfileManager::searchTemp(ir->getLop());
			regId = RegfileManager::searchTemp(ir->getLop());
		}
	}
	else {
		if (ir->getRopType() == INTTYPE) {
			r = RegfileManager::mappingTemp();
			addI(MIPS_LI, 0, regId = r->getId(), ir->getRopInt(), nullptr);
		}
		else if (ir->getRopType() == CHTYPE) {
			r = RegfileManager::mappingTemp();
			addI(MIPS_LI, 0, regId = r->getId(), (int)ir->getRop()->at(0), nullptr);
		}
		else if (ir->getRopType() == IDTYPE) {
			ti = TableTools::search(ir->getRop(), curScope);
			// in RegfileManager::mapping(ti, true), load = true, intend read.
			r = ti->getCache() == nullptr ? RegfileManager::mapping(ti, true) : ti->getCache();
			regId = r->getId();
		}
		else if (ir->getRopType() == TMPTYPE || ir->getRopType() == TMPTYPE_CH) {
			//regId = (ir->getRop())->find("$RET") != std::string::npos ? $v1 : RegfileManager::searchTemp(ir->getRop());
			regId = RegfileManager::searchTemp(ir->getRop());
		}
	}
	return regId;
}

void MipsGenerator::addDataAsciiz(std::string* label, std::string* strData) {
	dataSegment.push_back(new DataItem(label, nullptr,  _ASCIIZ, 0, strData));
}

void MipsGenerator::addDataWord(std::string* label, std::string* prototype, int data) {
	dataSegment.push_back(new DataItem(label, prototype, _WORD, data, nullptr));
}

void MipsGenerator::addDataSpace(std::string* label, std::string* prototype, int space) {
	dataSegment.push_back(new DataItem(label, prototype, __SPACE, space, nullptr));
}

void MipsGenerator::addR(int instr, int rs, int rt, int rd) {
	textSegment.push_back(new TextItem(instr, rs, rt, rd, 0, nullptr));
}

void MipsGenerator::addI(int instr, int rs, int rt, int immediate, std::string* label) {
	textSegment.push_back(new TextItem(instr, rs, rt, 0, immediate, label));
}

void MipsGenerator::addSyscall() {
	textSegment.push_back(new TextItem(MIPS_SYSCALL, 0, 0, 0, 0, nullptr));
}

void MipsGenerator::addLabel(std::string* label) {
	textSegment.push_back(new TextItem(MIPS_LABEL, 0, 0, 0, 0, label));
}

void MipsGenerator::addJ(int instr, std::string* label) {
	textSegment.push_back(new TextItem(instr, 0, 0, 0, 0, label));
}

void MipsGenerator::addB(int instr, int rs, int rt, std::string* label) {
	textSegment.push_back(new TextItem(instr, rs, rt, 0, 0, label));
}

void MipsGenerator::initLocals(int scope) {
	for (int i = 0; i < table.size(); i++) {
		if (table[i]->isSameScope(scope)) {
			if (table[i]->getType() == PARAM && table[i]->getOffset() <= 8) {
				Reg* reg = RegfileManager::mapping(table[i], false, $a1 + table[i]->getOffset() / 4);
				table[i]->setCache(reg);
				reg->setValid(true);
				reg->setLabel(table[i]->getLabel());
				reg->setDirty(true);
			}
			if (table[i]->getType() != PARAM) {
				if (table[i]->getHasInitialValue()) {
					if (table[i]->getDimension() == 0) {
						addI(MIPS_LI, 0, $a0, table[i]->getInitialValue(), nullptr);
						addI(MIPS_SW, $sp, $a0, table[i]->getOffset(), nullptr);
					}
					else {
						for (int j = 0; j < table[i]->getDim0() * table[i]->getDim1(); j++) {
							addI(MIPS_LI, 0, $a0, table[i]->getInitialValues()->at(j), nullptr);
							addI(MIPS_SW, $sp, $a0, table[i]->getOffset() + 4 * j, nullptr);
						}
					}
				}
			}
		}
	}
}

bool MipsGenerator::isJumpingBack(std::string* label) {
	for (int i = textSegment.size() - 1; i >= 0; i--) {
		if (textSegment[i]->getInstr() == MIPS_LABEL && *textSegment[i]->getLabel() == *label) {
			return true;
		}
	}
	return false;
}

void MipsGenerator::generate() {
	// Initialize Regfile Manager
	RegfileManager::init();
	// Add Global Consts and Vars to Data Segment
	for (int i = 0; i < table.size(); i++) {
		if (table[i]->isSameScope(0) && table[i]->getType() != FUNC) {
			if (table[i]->getDimension() == 0) {
				addDataWord(table[i]->getLabel(), table[i]->getName(), table[i]->getInitialValue());
			}
			else /* if (table[i]->getDimension() > 0) */{
				if (table[i]->getInitialValues() != nullptr) { // Initialized
					std::vector<int>* initialValues = table[i]->getInitialValues();
					addDataWord(table[i]->getLabel(), table[i]->getName(), initialValues->at(0));
					for (int j = 1; j < table[i]->getDim0() * table[i]->getDim1(); j++) {
						addDataWord(nullptr, table[i]->getName(), initialValues->at(j));
					}
				}
				else {
					addDataSpace(table[i]->getLabel(), 
						table[i]->getName(), table[i]->getDim0() * table[i]->getDim1() * 4);
				}
			}
		}
		else { break; }
	}
	addDataAsciiz(new std::string("endl"), new std::string("\\n"));

	// Add Text
	addI(MIPS_ADDI, $sp, $sp, -TableTools::getstackSpaceOfScope(new std::string("main")), nullptr);
	addJ(MIPS_JAL, new std::string("Func_main"));
	addJ(MIPS_J, new std::string("END"));

	int curScope = 0;
	TableItem* ti = nullptr;
	Reg* r = nullptr;
	int rs = 0, rt = 0;
	int arguments = 0;
	std::vector<IrItem*>::iterator it;
	for (it = IrList.begin(); it != IrList.end(); it++) {
		IrItem* ir = *it;
		switch (ir->getOp()) {
		case IR_FUNCDEF:
			RegfileManager::flush();
			TableTools::cacheFlush();
			curScope++;
			ti = TableTools::search(ir->getLop(), 0);
			addLabel(ti->getLabel());
			initLocals(curScope);
			break;
		case IR_PRINT:
			if (ir->getLopType() == STRTYPE) {
				std::string* strLabel = strConLabelGen();
				addDataAsciiz(strLabel, ir->getLop());
				addI(MIPS_LA, 0, $a0, 0, strLabel);
				addI(MIPS_LI, 0, $v0, 4, nullptr);
				addSyscall();
			}
			else if (ir->getLopType() == IDTYPE) {
				ti = TableTools::search(ir->getLop(), curScope);
				if (ti->getCache() == nullptr) {
					RegfileManager::mapping(ti, true, $a0);		// map ti to $a0, load immediately
				}
				else {
					addI(MIPS_ADDI, ti->getCache()->getId(), $a0, 0, nullptr);
				}
				if (ti->getRetType() == INT) {
					addI(MIPS_LI, 0, $v0, 1, nullptr);
				}
				else if (ti->getRetType() == CHAR) {
					addI(MIPS_LI, 0, $v0, 11, nullptr);
				}
				addSyscall();	// syscall
				addI(MIPS_LA, 0, $a0, 0, new std::string("endl"));
				addI(MIPS_LI, 0, $v0, 4, nullptr);
				addSyscall();
			}
			else if (ir->getLopType() == TMPTYPE) {
				rs = RegfileManager::searchTemp(ir->getLop());
				addR(MIPS_ADD, rs, $zero, $a0);
				addI(MIPS_LI, 0, $v0, 1, nullptr);
				addSyscall();
				addI(MIPS_LA, 0, $a0, 0, new std::string("endl"));
				addI(MIPS_LI, 0, $v0, 4, nullptr);
				addSyscall();
				RegfileManager::setInvalid(rs);
			}
			else if (ir->getLopType() == TMPTYPE_CH) {
				rs = RegfileManager::searchTemp(ir->getLop());
				addR(MIPS_ADD, rs, $zero, $a0);
				addI(MIPS_LI, 0, $v0, 11, nullptr);
				addSyscall();
				addI(MIPS_LA, 0, $a0, 0, new std::string("endl"));
				addI(MIPS_LI, 0, $v0, 4, nullptr);
				addSyscall();
				RegfileManager::setInvalid(rs);
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
			break;
		case IR_SCAN:
			ti = TableTools::search(ir->getLop(), curScope);
			if (ti->getRetType() == INT) {
				addI(MIPS_LI, 0, $v0, 5, nullptr);
			}
			else if (ti->getRetType() == CHAR) {
				addI(MIPS_LI, 0, $v0, 12, nullptr);
			}
			addSyscall();
			r = ti->getCache() == nullptr ? RegfileManager::mapping(ti) : ti->getCache();	// intent write
			r->setDirty(true);
			addI(MIPS_ADDI, $v0, r->getId(), 0, nullptr);
			/*if (ti->getScope() == 0) {
				addI(MIPS_SW, 0, $v0, 0, ti->getLabel());
			}
			else {
				addI(MIPS_SW, $sp, $v0, ti->getOffset(), nullptr);
			}*/
			break;
		case IR_ASSIGN:
			ti = TableTools::search(ir->getRes(), curScope);
			// in RegfileManager::mapping(ti), load = false, intend write.
			r = ti->getCache() == nullptr ? RegfileManager::mapping(ti) : ti->getCache();

			if (ir->getLopType() == INTTYPE) {
				addI(MIPS_LI, 0, r->getId(), ir->getLopInt(), nullptr);
			}
			else if (ir->getLopType() == CHTYPE) {
				addI(MIPS_LI, 0, r->getId(), (int)ir->getLop()->at(0), nullptr);
			}
			else if (ir->getLopType() == IDTYPE) {
				TableItem* ti2 = TableTools::search(ir->getLop(), curScope);
				Reg* r2 = ti2->getCache() == nullptr ? RegfileManager::mapping(ti2, true) : ti2->getCache();
				addI(MIPS_ADDI, r2->getId(), r->getId(), 0, nullptr);
			}
			else if (ir->getLopType() == TMPTYPE || ir->getLopType() == TMPTYPE_CH) {
				/*if ((ir->getLop())->find("$RET") != std::string::npos) {
					addI(MIPS_ADDI, $v1, r->getId(), 0, nullptr);
				}
				else {*/
					int rt = RegfileManager::searchTemp(ir->getLop());
					addI(MIPS_ADDI, rt, r->getId(), 0, nullptr);
					RegfileManager::setInvalid(rt);
				//}
			}

			r->setDirty(true);
			break;
		case IR_ADD:
		case IR_SUB:
		case IR_MUL:
		case IR_DIV:
			rs = getRegL0R1(ir, curScope, 0);
			rt = getRegL0R1(ir, curScope, 1);

			if (ir->getRes()->at(0) != '$') {
				ti = TableTools::search(ir->getRes(), curScope);
				// in RegfileManager::mapping(ti), load = false, intend write.
				r = ti->getCache() == nullptr ? RegfileManager::mapping(ti) : ti->getCache();
				r->setDirty(true);
			}
			else {
				r = RegfileManager::mappingTemp(ir->getRes());
			}

			addR(ir->getOp(), rs, rt, r->getId());
			if (ir->getLopType() == TMPTYPE || ir->getLopType() == TMPTYPE_CH || 
				ir->getLopType() == INTTYPE || ir->getLopType() == CHTYPE) {
				RegfileManager::setInvalid(rs);
			}
			if (ir->getRopType() == TMPTYPE || ir->getRopType() == TMPTYPE_CH ||
				ir->getRopType() == INTTYPE || ir->getRopType() == CHTYPE) {
				RegfileManager::setInvalid(rt);
			}
			break;
		case IR_RET:
			RegfileManager::writeSBack();
			if (ir->getLopType() == INTTYPE) {
				addI(MIPS_LI, 0, $v1, ir->getLopInt(), nullptr);
			}
			else if (ir->getLopType() == CHTYPE) {
				addI(MIPS_LI, 0, $v1, (int)ir->getLop()->at(0), nullptr);
			}
			else if (ir->getLopType() == IDTYPE) {
				ti = TableTools::search(ir->getLop(), curScope);
				r = ti->getCache() == nullptr ? RegfileManager::mapping(ti, true) : ti->getCache();
				addI(MIPS_ADDI, r->getId(), $v1, 0, nullptr);
			}
			else if (ir->getLopType() == TMPTYPE || ir->getLopType() == TMPTYPE_CH) {
				/*if ((ir->getLop())->find("$RET") != std::string::npos) {		// "return func();" in a function 
					// addI(MIPS_ADDI, $v1, $v1, 0, nullptr);
				}
				else {*/
					rs = RegfileManager::searchTemp(ir->getLop());
					addI(MIPS_ADDI, rs, $v1, 0, nullptr);
					RegfileManager::setInvalid(rs);
				//}
			}
			addR(MIPS_JR, $ra, 0, 0);
			break;
		case IR_PRECALL:
			addI(MIPS_ADDI, $sp, $fp, 0, nullptr);
			RegfileManager::writeSBack();
			RegfileManager::saveEnv();
			addI(MIPS_ADDI, $sp, $sp, -TableTools::getstackSpaceOfScope(ir->getLop()), nullptr);
			arguments = 0;
			break;
		case IR_PUSH:
			if (arguments < 3) {	// Save first 3 args to $a1, $a2, $a3
				rt = arguments + $a1;
				if (ir->getLopType() == INTTYPE) {
					addI(MIPS_LI, 0, rt, ir->getLopInt(), nullptr);
				}
				else if (ir->getLopType() == CHTYPE) {
					addI(MIPS_LI, 0, rt, (int)ir->getLop()->at(0), nullptr);
				}
				else if (ir->getLopType() == IDTYPE) {
					ti = TableTools::search(ir->getLop(), curScope);
					r = ti->getCache() == nullptr ? RegfileManager::mapping(ti, true, $a0, $fp) : ti->getCache();
					addI(MIPS_ADDI, r->getId(), rt, 0, nullptr);
				}
				else if (ir->getLopType() == TMPTYPE || ir->getLopType() == TMPTYPE_CH) {
					/*if ((ir->getLop())->find("$RET") != std::string::npos) {
						addI(MIPS_ADDI, $v1, rt, 0, nullptr);
					}
					else {*/
						rs = RegfileManager::searchTemp(ir->getLop());
						addI(MIPS_ADDI, rs, rt, 0, nullptr);
						RegfileManager::setInvalid(rs);
					//}
				}
			}
			else {	// Save the other args to stack
				if (ir->getLopType() == INTTYPE) {
					addI(MIPS_LI, 0, $a0, ir->getLopInt(), nullptr);
					addI(MIPS_SW, $sp, $a0, 4 * arguments, nullptr);
				}
				else if (ir->getLopType() == CHTYPE) {
					addI(MIPS_LI, 0, $a0, (int)ir->getLop()->at(0), nullptr);
					addI(MIPS_SW, $sp, $a0, 4 * arguments, nullptr);
				}
				else if (ir->getLopType() == IDTYPE) {
					ti = TableTools::search(ir->getLop(), curScope);
					r = ti->getCache() == nullptr ? RegfileManager::mapping(ti, true, $a0, $fp) : ti->getCache();
					addI(MIPS_SW, $sp, r->getId(), 4 * arguments, nullptr);
				}
				else if (ir->getLopType() == TMPTYPE || ir->getLopType() == TMPTYPE_CH) {
					/*if ((ir->getLop())->find("$RET") != std::string::npos) {
						addI(MIPS_SW, $sp, $v1, 4 * arguments, nullptr);
					}
					else {*/
						rt = RegfileManager::searchTemp(ir->getLop());
						addI(MIPS_SW, $sp, rt, 4 * arguments, nullptr);
						RegfileManager::setInvalid(rt);
					//}
				}
			}
			arguments++;
			break;
		case IR_CALL:
			ti = TableTools::search(ir->getLop(), 0);
			addJ(MIPS_JAL, ti->getLabel());
			addI(MIPS_ADDI, $sp, $sp, TableTools::getstackSpaceOfScope(ir->getLop()), nullptr);
			RegfileManager::restoreEnv();
			if (ti->getRetType() != VOID) {		// Has Return-Value
				if (ir->getRes() != nullptr) {
					r = RegfileManager::mappingTemp(ir->getRes());
					addI(MIPS_ADDI, $v1, r->getId(), 0, nullptr);
				}
			}
			break;
		case IR_LABEL:
			//if (ir->getRes()->find("While") != std::string::npos ||
			//	ir->getRes()->find("For") != std::string::npos) {
				RegfileManager::writeAllBack();
			//}
			addLabel(ir->getRes());
			break;
		case IR_EQL:
		case IR_NEQ:
		case IR_LSS:
		case IR_LEQ:
		case IR_GRE:
		case IR_GEQ:
			//if (isJumpingBack(ir->getRes())) {
				RegfileManager::writeAllBack();
			//}                      
			rs = getRegL0R1(ir, curScope, 0);
			rt = getRegL0R1(ir, curScope, 1);
			it++;	ir = *it;
			addB(bInstrJudger((*(it - 1))->getOp(), ir->getOp()), rs, rt, ir->getRes());
			if (
				/*!(  //	To ensure not set invalid for left op of first while judge.
					(*(it + 1))->getOp() == IR_LABEL &&
					(*(it + 1))->getRes()->find("While") != std::string::npos
				) && */
				!(  //	To ensure not set invalid for switch key.
					(*(it + 1))->getOp() == IR_EQL                         && 
					(*(it + 1))->getLopType() == (*(it - 1))->getLopType() &&
					(*(it + 1))->getLopInt()  ==  (*(it - 1))->getLopInt() &&
					*(*(it + 1))->getLop()    ==    *(*(it - 1))->getLop()
				)) {
				if ((*(it - 1))->getLopType() == TMPTYPE || (*(it - 1))->getLopType() == TMPTYPE_CH ||
					(*(it - 1))->getLopType() == INTTYPE || (*(it - 1))->getLopType() == CHTYPE) {
					RegfileManager::setInvalid(rs);
				}	
			}
			/*if (!(  //	To ensure not set invalid for left op of first while judge.
				(*(it + 1))->getOp() == IR_LABEL &&
				(*(it + 1))->getRes()->find("While") != std::string::npos
				)) {*/
				if ((*(it - 1))->getRopType() == TMPTYPE || (*(it - 1))->getRopType() == TMPTYPE_CH ||
					(*(it - 1))->getRopType() == INTTYPE || (*(it - 1))->getRopType() == CHTYPE) {
					RegfileManager::setInvalid(rt);
				}
			//}
			break;
		case IR_GOTO:
			//if (isJumpingBack(ir->getRes())) {
				RegfileManager::writeAllBack();
			//}
			addJ(MIPS_J, ir->getRes());
			break;
		case IR_ARRAYGET:
			if (ir->getRes()->at(0) != '$') {
				ti = TableTools::search(ir->getRes(), curScope);
				// in RegfileManager::mapping(ti), load = false, intend write.
				r = ti->getCache() == nullptr ? RegfileManager::mapping(ti) : ti->getCache();
				r->setDirty(true);
			}
			else {
				r = RegfileManager::mappingTemp(ir->getRes());
			}

			ti = TableTools::search(ir->getLop(), curScope);
			rt = getRegL0R1(ir, curScope, 1);
			rs = RegfileManager::mappingTemp()->getId();
			addI(MIPS_SLL, rt, rs, 2, nullptr);	// Actually, sll is a R-type Instruction
			if (ti->getScope() == 0) {
				addI(MIPS_LW, rs, r->getId(), 0, ti->getLabel());
			}
			else {
				addI(MIPS_ADDI, rs, rs, ti->getOffset(), nullptr);
				addR(MIPS_ADD, rs, $sp, rs);
				addI(MIPS_LW, rs, r->getId(), 0, nullptr);
			}
			
			RegfileManager::setInvalid(rs);
			if (ir->getRopType() == TMPTYPE || ir->getRopType() == TMPTYPE_CH ||
				ir->getRopType() == INTTYPE || ir->getRopType() == CHTYPE) {
				RegfileManager::setInvalid(rt);
			}
			break;
		case IR_ARRAYSET:
			rs = getRegL0R1(ir, curScope, 0);

			ti = TableTools::search(ir->getRes(), curScope);
			rt = getRegL0R1(ir, curScope, 1);
			r = RegfileManager::mappingTemp();
			addI(MIPS_SLL, rt, r->getId(), 2, nullptr);	// Actually, sll is a R-type Instruction
			if (ti->getScope() == 0) {
				addI(MIPS_SW, r->getId(), rs, 0, ti->getLabel());
			}
			else {
				addI(MIPS_ADDI, r->getId(), r->getId(), ti->getOffset(), nullptr);
				addR(MIPS_ADD, r->getId(), $sp, r->getId());
				addI(MIPS_SW, r->getId(), rs, 0, nullptr);
			}

			RegfileManager::setInvalid(r->getId());
			if (ir->getLopType() == TMPTYPE || ir->getLopType() == TMPTYPE_CH ||
				ir->getLopType() == INTTYPE || ir->getLopType() == CHTYPE) {
				RegfileManager::setInvalid(rs);
			}
			if (ir->getRopType() == TMPTYPE || ir->getRopType() == TMPTYPE_CH ||
				ir->getRopType() == INTTYPE || ir->getRopType() == CHTYPE) {
				RegfileManager::setInvalid(rt);
			}
			break;
		case IR_SLL:
			assert(ir->getRopType() == INTTYPE);
			rs = getRegL0R1(ir, curScope, 0);

			if (ir->getRes()->at(0) != '$') {
				ti = TableTools::search(ir->getRes(), curScope);
				// in RegfileManager::mapping(ti), load = false, intend write.
				r = ti->getCache() == nullptr ? RegfileManager::mapping(ti) : ti->getCache();
				r->setDirty(true);
			}
			else {
				r = RegfileManager::mappingTemp(ir->getRes());
			}
			addI(MIPS_SLL, rs, r->getId(), ir->getRopInt(), nullptr);
			break;
		}
	}

	addLabel(new std::string("END"));
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
			if ((*dataIt)->getLabel() != nullptr) {
				out << "\t" << *(*dataIt)->getLabel() << ": .word " << (*dataIt)->getData() << std::endl;
			}
			else { // For array, while not_first elements don't need a label.
				out << "\t\t" << ".word " << (*dataIt)->getData() << std::endl;
			}
		}
		else if ((*dataIt)->getDataType() == __SPACE) {
			out << "\t" << *(*dataIt)->getLabel() << ": .space " << (*dataIt)->getData() << std::endl;
		}
	}

	out << ".text" << std::endl;
	std::vector<TextItem*>::iterator textIt;
	for (textIt = textSegment.begin(); textIt != textSegment.end(); ++textIt) {
		switch ((*textIt)->getInstr()) {
		case MIPS_LI:
			out << "\tli " << regstr[(*textIt)->getRt()] << ", " << (*textIt)->getImmediate() << std::endl;
			break;
		case MIPS_LA:
			out << "\tla " << regstr[(*textIt)->getRt()] << ", " << *(*textIt)->getLabel() << std::endl;
			break;
		case MIPS_SYSCALL:
			out << "\tsyscall" << std::endl;
			break;
		case MIPS_SW:
		case MIPS_LW:
			if ((*textIt)->getLabel() != nullptr) {
				out << "\t" << mipsInstructions[(*textIt)->getInstr()] << " " <<
					regstr[(*textIt)->getRt()] << ", " << *(*textIt)->getLabel() << 
					"(" << regstr[(*textIt)->getRs()] << ")" << std::endl;
			}
			else {
				out << "\t" << mipsInstructions[(*textIt)->getInstr()] << " " <<
					regstr[(*textIt)->getRt()] << ", " << (*textIt)->getImmediate() <<
					"(" << regstr[(*textIt)->getRs()] << ")" << std::endl;
			}
			break;
		case MIPS_ADD:
		case MIPS_SUB:
		case MIPS_MUL:
		case MIPS_DIV:
			out << "\t" << mipsInstructions[(*textIt)->getInstr()] << " " <<
				regstr[(*textIt)->getRd()] << ", " <<
				regstr[(*textIt)->getRs()] << ", " <<
				regstr[(*textIt)->getRt()] << std::endl;
			break;
		case MIPS_LABEL:
			out << *(*textIt)->getLabel() << ":" << std::endl;
			break;
		case MIPS_ADDI:
			out << "\t" << mipsInstructions[(*textIt)->getInstr()] << " " <<
				regstr[(*textIt)->getRt()] << ", " <<
				regstr[(*textIt)->getRs()] << ", " <<
				(*textIt)->getImmediate() << std::endl;
			break;
		case MIPS_J:
		case MIPS_JAL:
			out << "\t" << mipsInstructions[(*textIt)->getInstr()] << " " 
				<< *(*textIt)->getLabel() << std::endl;
			break;
		case MIPS_JR:
			out << "\t" << mipsInstructions[(*textIt)->getInstr()] << " "
				<< regstr[(*textIt)->getRs()] << std::endl;
			break;
		case MIPS_BEQ:
		case MIPS_BNE:
		case MIPS_BLT:
		case MIPS_BLE:
		case MIPS_BGT:
		case MIPS_BGE:
			out << "\t" << mipsInstructions[(*textIt)->getInstr()] << " "
				<< regstr[(*textIt)->getRs()] << ", "
				<< regstr[(*textIt)->getRt()] << ", "
				<< *(*textIt)->getLabel() << std::endl;
			break;
		case MIPS_SLL:
			out << "\t" << mipsInstructions[(*textIt)->getInstr()] << " "
				<< regstr[(*textIt)->getRt()] << ", "
				<< regstr[(*textIt)->getRs()] << ", "
				<< (*textIt)->getImmediate() << std::endl;
		}
	}
}