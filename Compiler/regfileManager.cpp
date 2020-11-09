#include "regfileManager.h"

Reg* regfile[32];

Reg::Reg(int id) {
	this->id = id;
	this->label = nullptr;
	this->valid = false;
	this->dirty = false;
}

int Reg::getId() {
	return id;
}

std::string* Reg::getLabel() {
	return label;
}

bool Reg::getValid() {
	return valid;
}

bool Reg::getDirty() {
	return dirty;
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

void RegfileManager::init() {
	for (int i = 0; i < 32; i++) {
		regfile[i] = new Reg(i);
	}
}

Reg* RegfileManager::getTempReg() {
	static int i = 0;
	Reg* reg = i <= 7 ? regfile[i + 8] : regfile[i + 16];
	i++; if (i > 9) { i -= 10; } // i = (i + 1) % 10;
	// TODO: Write Back to Memory
	if (reg->getValid() && reg->getDirty()) {

	}
	reg->setValid(true);
	return reg;
}

Reg* RegfileManager::getSavedReg() {
	static int i = 0;
	Reg* reg = regfile[i + 16];
	i++; if (i > 7) { i -= 8; } // i = (i + 1) % 8;
	// TODO: Write Back to Memory
	if (reg->getValid() && reg->getDirty()) {

	}
	reg->setValid(true);
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
	return 0;
}