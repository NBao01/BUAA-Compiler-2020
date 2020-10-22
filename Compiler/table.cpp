#include "table.h"
#include <cassert>
#include "symbols.h"
#include "tokens.h"
#include <iostream>
#include "error.h"

std::vector<TableItem*> table;

int TableItem::scope_i = 0;

int TableTools::it_prev= 0;
std::string* TableTools::name = nullptr;
int TableTools::type = -1;
int TableTools::retType = -1;
int TableTools::dimension = -1;

TableItem::TableItem(std::string* name, int type, int retType, int scope) {
	this->name = name;
	this->type = type;
	this->retType = retType;
	this->scope = scope;
	dimension = 0;
	paramNum = 0;
	paramsRetType = nullptr;
}

TableItem* TableItem::newConstTableItem(std::string* name, int type, int retType) {
	assert(type == CONST && retType != VOID);
	return new TableItem(name, type, retType, scope_i);
}

TableItem* TableItem::newVarTableItem(std::string* name, int type, int retType, int dimension) {
	assert(type == VAR && retType != VOID);
	TableItem* ti = new TableItem(name, type, retType, scope_i);
	ti->setDimension(dimension);
	return ti;
}

TableItem* TableItem::newFuncTableItem(std::string* name, int type, int retType) {
	assert(type == FUNC);
	TableItem* ti = new TableItem(name, type, retType, 0);
	//ti->setParams(paramNum, params);
	scope_i++;
	return ti;
}

TableItem* TableItem::newParamTableItem(std::string* name, int type, int retType) {
	assert(type == PARAM);
	return new TableItem(name, type, retType, scope_i);
}

void TableItem::setDimension(int dimension) {
	this->dimension = dimension;
}

void TableItem::setParams(int paramNum, std::vector<int>* paramsRetType) {
	this->paramNum = paramNum;
	this->paramsRetType = paramsRetType;
}

bool TableItem::isSameScope(int curScope) {
	return scope == curScope;
}

bool TableItem::isSameName(std::string* name) {
	return *(this->name) == *name;
}

// Add consts to the table, add error_b to errorlist if there is any redefined consts
void TableTools::addConsts(int it) {
	type = CONST;
	for (int i = it_prev; i < it; i++) {
		// std::cout << tokens[wordlist[i]->getType()] << " " << wordlist[i]->getWord() << std::endl;
		if (wordlist[i]->getType() == INTTK) {
			retType = INT;
		}
		else if (wordlist[i]->getType() == CHARTK) {
			retType = CHAR;
		}

		if (wordlist[i]->getType() == IDENFR) {
			name = &wordlist[i]->getWord();

			// ERROR_B JUDGER
			for (int i = table.size() - 1; i >= 0; i--) {
				if (table[i]->isSameScope(TableItem::scope_i) && table[i]->isSameName(name)) {
					ErrorHandler::addErrorItem(ERROR_B, wordlist[i]->getLine());
				}
			}
			// ERROR_B JUDGER END

			table.push_back(TableItem::newConstTableItem(name, type, retType));
		}
	}
	it_prev = it;
}

// Add variables to the table, add error_b to errorlist if there is any redefined variables
void TableTools::addVars(int it) {
	type = VAR;
	dimension = 0;
	for (int i = it_prev; i < it; i++) {
		// std::cout << tokens[wordlist[i]->getType()] << " " << wordlist[i]->getWord() << std::endl;
		if (wordlist[i]->getType() == INTTK) {
			retType = INT;
		}
		else if (wordlist[i]->getType() == CHARTK) {
			retType = CHAR;
		}

		if (wordlist[i]->getType() == IDENFR) {
			name = &wordlist[i]->getWord();

			// ERROR_B JUDGER
			for (int i = table.size() - 1; i >= 0; i--) {
				if (table[i]->isSameScope(TableItem::scope_i) && table[i]->isSameName(name)) {
					ErrorHandler::addErrorItem(ERROR_B, wordlist[i]->getLine());
				}
			}
			// ERROR_B JUDGER END
		}

		if (wordlist[i]->getType() == RBRACK) {
			dimension++;
		}

		if (wordlist[i]->getType() == COMMA || wordlist[i]->getType() == SEMICN) {
			table.push_back(TableItem::newVarTableItem(name, type, retType, dimension));
			dimension = 0;
		}
	}
	it_prev = it;
}

// Add function (just one func) and their params to the table, add error_b to errorlist if there is any redefined identifer
void TableTools::addFunc(int it) {
	type = FUNC;
	TableItem* funcItem = nullptr;
	int paramsNum = 0;
	std::vector<int>* paramsRetType = new std::vector<int>();
	bool retTypeIsDetected = false, funcIdentifierIsDetected = false, paramsAreAllDetected = false;
	for (int i = it_prev; i < it; i++) {
		// std::cout << tokens[wordlist[i]->getType()] << " " << wordlist[i]->getWord() << std::endl;
		if (!retTypeIsDetected) {
			if (wordlist[i]->getType() == VOIDTK) {
				retType = VOID;
				retTypeIsDetected = true;
			}
			else if (wordlist[i]->getType() == INTTK) {
				retType = INT;
				retTypeIsDetected = true;
			}
			else if (wordlist[i]->getType() == CHARTK) {
				retType = CHAR;
				retTypeIsDetected = true;
			}
		} // Determine the return type of the function
		else if (!funcIdentifierIsDetected) {
			if (wordlist[i]->getType() == IDENFR || MAINTK) {
				name = &wordlist[i]->getWord();
				funcItem = TableItem::newFuncTableItem(name, type, retType);
				table.push_back(funcItem);
				funcIdentifierIsDetected = true;
			}
		} // Determine the function name
		else if (!paramsAreAllDetected) {
			type = PARAM;
			if (wordlist[i]->getType() == INTTK) {
				retType = INT;
				paramsRetType->push_back(retType);
			}
			else if (wordlist[i]->getType() == CHARTK) {
				retType = CHAR;
				paramsRetType->push_back(retType);
			}

			if (wordlist[i]->getType() == IDENFR) {
				name = &wordlist[i]->getWord();

				// ERROR_B JUDGER
				for (int i = table.size() - 1; i >= 0; i--) {
					if (table[i]->isSameScope(TableItem::scope_i) && table[i]->isSameName(name)) {
						ErrorHandler::addErrorItem(ERROR_B, wordlist[i]->getLine());
					}
				}
				// ERROR_B JUDGER END
				
				table.push_back(TableItem::newParamTableItem(name, type, retType));
				paramsNum++;
			}

			if (wordlist[i]->getType() == RPARENT) {
				paramsAreAllDetected = true;
			}
		} // Determine all parameters of the function
	}
	assert(funcItem != nullptr);
	funcItem->setParams(paramsNum, paramsRetType);
	it_prev = it;
}