#include "regfileManager.h"
#include "mipsDefinitions.h"
#include "mips.h"
#include <iostream>
#include <cassert>

Reg* regfile[32];

Reg::Reg(int id) {
	this->id = id;
	this->label = nullptr;
	this->valid = false;
	this->dirty = false;
	this->temp = false;
	this->inStack = false;
}

int Reg::getId() {
	return id;
}

std::string* Reg::getLabel() {
	return label;
}

bool Reg::isValid() {
	return valid;
}

bool Reg::isDirty() {
	return dirty;
}

bool Reg::isTemp() {
	return temp;
}

bool Reg::isInStack() {
	return inStack;
}

void Reg::setLabel(std::string* label) {
	this->label = label;
}

void Reg::setValid(bool valid) {
	this->valid = valid;
}

void Reg::setDirty(bool dirty) {
	this->dirty = dirty;
}

void Reg::setTemp(bool temp) {
	this->temp = temp;
}

void Reg::setInStack(bool inStack) {
	this->inStack = inStack;
}

void RegfileManager::init() {
	for (int i = 0; i < 32; i++) {
		regfile[i] = new Reg(i);
	}
}

Reg* RegfileManager::getTempReg() {
	/*static int i = 0;
	Reg* reg = i <= 7 ? regfile[i + 8] : regfile[i + 16];
	i++; if (i > 9) { i -= 10; } // i = (i + 1) % 10;*/

	int regId = -1;
	for (int i = 0; i < 10; i++) {
		int j = i <= 7 ? i + 8 : i + 16;
		if (!regfile[j]->isValid()) {
			regId = j;
			break;
		}
	}
	if (regId == -1) {	// No invalid regs
		static int roundRobin = 0;
		do {
			regId = roundRobin <= 7 ? roundRobin + 8 : roundRobin + 16;
			roundRobin = (roundRobin + 1) % 10;
		} while (!regfile[regId]->isTemp());
		// You can't use a valid temp reg, for the value in the reg will disappear!
	}

	Reg* reg = regfile[regId];
	if (reg->isValid() && !reg->isTemp()) {
		// TODO: Write Back to Memory
		TableItem* ti = TableTools::searchByLabel(reg->getLabel());
		if (reg->isDirty()) {
			TableItem* ti = TableTools::searchByLabel(reg->getLabel());
			if (ti->getScope() == 0) {
				MipsGenerator::addI(MIPS_SW, 0, reg->getId(), 0, ti->getLabel());
			}
			else {
				MipsGenerator::addI(MIPS_SW, $sp, reg->getId(), ti->getOffset(), nullptr);
			}
		}
		ti->setCache(nullptr);
	}
	reg->setValid(true);
	reg->setDirty(false);
	reg->setTemp(false);
	reg->setLabel(nullptr);
	return reg;
}

Reg* RegfileManager::getSavedReg() {
	static int i = 0;
	Reg* reg = regfile[i + 16];
	i++; if (i > 7) { i -= 8; } // i = (i + 1) % 8;
	// TODO: Write Back to Memory
	if (reg->isValid()) {
		TableItem* ti = TableTools::searchByLabel(reg->getLabel());
		if (reg->isDirty()) {
			if (ti->getScope() == 0) {
				MipsGenerator::addI(MIPS_SW, 0, reg->getId(), 0, ti->getLabel());
			}
			else {
				MipsGenerator::addI(MIPS_SW, $sp, reg->getId(), ti->getOffset(), nullptr);
			}
		}
		ti->setCache(nullptr);
	}
	reg->setValid(true);
	reg->setDirty(false);
	reg->setTemp(false);
	reg->setLabel(nullptr);
	return reg;
}

/* Map TableItem ti to a register.
 * load = true  : load the value to register immediately, intent to read.
 * load = false : no loading, intent to write.
 * specificReg is not 0 : specify a certain register to the tableItem, useful for condition such as printf
 * sp is the base register, is $sp mostly, is $fp when in precall
 */
Reg* RegfileManager::mapping(TableItem* ti, bool load, int specificReg, int sp) {
	Reg* reg = nullptr;
	if (ti->getScope() == 0) {
		reg = specificReg == 0 ? RegfileManager::getSavedReg() : regfile[specificReg];
		if (load) {
			MipsGenerator::addI(MIPS_LW, 0, reg->getId(), 0, ti->getLabel());
		}
	}
	else {
		reg = specificReg == 0 ? RegfileManager::getTempReg() : regfile[specificReg];
		if (load) {
			MipsGenerator::addI(MIPS_LW, sp, reg->getId(), ti->getOffset(), nullptr);
		}
	}

	if (specificReg == 0) {		// mapped register unspecified
		ti->setCache(reg);
		reg->setLabel(ti->getLabel());
	}
	return reg;
}

// Reg for intermediate result
Reg* RegfileManager::mappingTemp(std::string* label) {
	Reg* reg = RegfileManager::getTempReg();
	reg->setLabel(label);
	reg->setTemp(true);
	return reg;
}

// Reg for constant, No-Name-Reg, Use at once.
Reg* RegfileManager::mappingTemp() {
	Reg* reg = RegfileManager::getTempReg();
	reg->setLabel(nullptr);
	reg->setValid(true);
	reg->setTemp(true);
	return reg;
}

int RegfileManager::searchTemp(std::string* label) {
	int j;
	for (int i = 0; i < 10; i++) {
		j = i <= 7 ? i + 8 : i + 16;
		if (regfile[j]->getLabel() && *label == *regfile[j]->getLabel()) {
			return j;
		}
	}
	std::cout << "In regfileManager.cpp, line 150: No Such Temp Register!" << std::endl;
	return 0;
}

void RegfileManager::writeAllBack() {
	for (int i = 0; i < 10; i++) {
		Reg* reg = regfile[i <= 7 ? i + 8 : i + 16];
		if (!reg->isTemp() && reg->isValid()) {
			TableItem* ti = TableTools::searchByLabel(reg->getLabel());
			if (reg->isDirty()) {
				if (ti->getScope() == 0) {
					MipsGenerator::addI(MIPS_SW, 0, reg->getId(), 0, ti->getLabel());
				}
				else {
					MipsGenerator::addI(MIPS_SW, $sp, reg->getId(), ti->getOffset(), nullptr);
				}
			}
			ti->setCache(nullptr);
			reg->setValid(false);
		}
	}
	for (int i = 16; i < 24; i++) {
		Reg* reg = regfile[i];
		if (reg->isValid()) {
			TableItem* ti = TableTools::searchByLabel(reg->getLabel());
			if (reg->isDirty()) {
				if (ti->getScope() == 0) {
					MipsGenerator::addI(MIPS_SW, 0, reg->getId(), 0, ti->getLabel());
				}
				else {
					MipsGenerator::addI(MIPS_SW, $sp, reg->getId(), ti->getOffset(), nullptr);
				}
			}
			ti->setCache(nullptr);
			reg->setValid(false);
		}
	}
}

void RegfileManager::writeSBack() {
	for (int i = 16; i < 24; i++) {
		Reg* reg = regfile[i];
		if (reg->isValid()) {
			TableItem* ti = TableTools::searchByLabel(reg->getLabel());
			if (reg->isDirty()) {
				if (ti->getScope() == 0) {
					MipsGenerator::addI(MIPS_SW, 0, reg->getId(), 0, ti->getLabel());
				}
				else {
					MipsGenerator::addI(MIPS_SW, $sp, reg->getId(), ti->getOffset(), nullptr);
				}
			}
			ti->setCache(nullptr);
			reg->setValid(false);
		}
	}
}

void RegfileManager::flush() {
	for (int i = 0; i < 10; i++) {
		regfile[i <= 7 ? i + 8 : i + 16]->setValid(false);
	}
	for (int i = 16; i < 24; i++) {
		regfile[i]->setValid(false);
	}
}

void RegfileManager::saveEnv() {
	int stackSpace = 4;		//	Reserve space for $ra
	for (int i = 5; i < 26; i++) {
		if (regfile[i]->isValid()) {
			// This reg will not be invalid when push arguments.
			stackSpace += 4;
			regfile[i]->setInStack(true);
		}
	}
	MipsGenerator::addI(MIPS_ADDI, $sp, $sp, -stackSpace, nullptr);
	int curOffset = 0;
	for (int i = 5; i < 26; i++) {
		if (regfile[i]->isValid()) {
			MipsGenerator::addI(MIPS_SW, $sp, i, curOffset, nullptr);
			curOffset += 4;
			regfile[i]->setInStack(true);
		}
	}
	assert(curOffset == stackSpace - 4);
	MipsGenerator::addI(MIPS_SW, $sp, $ra, curOffset, nullptr);

	/*
	MipsGenerator::addI(MIPS_ADDI, $sp, $sp, -88, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $a1, 0, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $a2, 4, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $a3, 8, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $t0, 12, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $t1, 16, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $t2, 20, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $t3, 24, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $t4, 28, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $t5, 32, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $t6, 36, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $t7, 40, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $t8, 44, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $t9, 48, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $s0, 52, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $s1, 56, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $s2, 60, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $s3, 64, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $s4, 68, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $s5, 72, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $s6, 76, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $s7, 80, nullptr);
	MipsGenerator::addI(MIPS_SW, $sp, $ra, 84, nullptr);
	*/
}

void RegfileManager::restoreEnv() {
	int curOffset = 0;
	for (int i = 5; i < 26; i++) {
		//if (regfile[i]->isValid()) {
		if (regfile[i]->isInStack()) {
			MipsGenerator::addI(MIPS_LW, $sp, i, curOffset, nullptr);
			curOffset += 4;
			regfile[i]->setInStack(false);
		}
	}
	MipsGenerator::addI(MIPS_LW, $sp, $ra, curOffset, nullptr);
	MipsGenerator::addI(MIPS_ADDI, $sp, $sp, curOffset + 4, nullptr);

	/*
	MipsGenerator::addI(MIPS_LW, $sp, $a1, 0, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $a2, 4, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $a3, 8, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $t0, 12, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $t1, 16, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $t2, 20, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $t3, 24, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $t4, 28, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $t5, 32, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $t6, 36, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $t7, 40, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $t8, 44, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $t9, 48, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $s0, 52, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $s1, 56, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $s2, 60, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $s3, 64, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $s4, 68, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $s5, 72, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $s6, 76, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $s7, 80, nullptr);
	MipsGenerator::addI(MIPS_LW, $sp, $ra, 84, nullptr);
	MipsGenerator::addI(MIPS_ADDI, $sp, $sp, 88, nullptr);
	*/
}

void RegfileManager::setInvalid(int regId) {
	regfile[regId]->setValid(false);
}