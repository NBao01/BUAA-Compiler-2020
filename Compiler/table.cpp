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
	std::string a(""), b("");
	for (int i = 0; i < name->size(); i++) {
		a += tolower(name->at(i));
	}
	for (int i = 0; i < this->name->size(); i++) {
		b += tolower(this->name->at(i));
	}
	return a == b;
}

int TableItem::getParamNum() {
	return paramNum;
}

int TableItem::getType() {
	return type;
}

int TableItem::getRetType() {
	return retType;
}

std::vector<int>* TableItem::getParamsRetType() {
	return paramsRetType;
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
			for (int j = table.size() - 1; j >= 0; j--) {
				if (table[j]->isSameScope(TableItem::scope_i) && table[j]->isSameName(name)) {
					ErrorHandler::addErrorItem(ERROR_B, wordlist[i]->getLine());
					break;
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
			for (int j = table.size() - 1; j >= 0; j--) {
				if (table[j]->isSameScope(TableItem::scope_i) && table[j]->isSameName(name)) {
					ErrorHandler::addErrorItem(ERROR_B, wordlist[i]->getLine());
					break;
				}
			}
			// ERROR_B JUDGER END
		}

		if (wordlist[i]->getType() == RBRACK) {
			dimension++;
		}

		if (wordlist[i]->getType() == LBRACE) {
			i++;
			while (wordlist[i]->getType() != RBRACE) {
				if (wordlist[i]->getType() == LBRACE) {
					while (wordlist[i]->getType() != RBRACE) {
						i++;
					}
				}
				i++;
			}
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

				// ERROR_B JUDGER
				for (int j = table.size() - 1; j >= 0; j--) {
					if (table[j]->isSameScope(TableItem::scope_i) && table[j]->isSameName(name)) {
						ErrorHandler::addErrorItem(ERROR_B, wordlist[i]->getLine());
						break;
					}
				}
				// ERROR_B JUDGER END

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
				for (int j = table.size() - 1; j >= 0; j--) {
					if (table[j]->isSameScope(TableItem::scope_i) && table[j]->isSameName(name)) {
						ErrorHandler::addErrorItem(ERROR_B, wordlist[i]->getLine());
						break;
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

// Return true if the func is void func, return false if the func is non-void func or not exists.
bool TableTools::isVoidFunc(std::string* word) {
	for (int i = table.size() - 1; i >= 0; i--) {
		if (table[i]->getType() == FUNC &&
			table[i]->getRetType() == VOID &&
			table[i]->isSameName(name)) {
			return true;
		}
	}
	return false;
}

// Return true if the expression is Char Type
bool TableTools::isCharType(SymbolNode* node) {
	assert(node->getType() == 表達式);
	if (node->getChildren()->size() == 1) {
		node = node->getChildren()->at(0);
		if (node->getType() == 項 && node->getChildren()->size() == 1) {
			node = node->getChildren()->at(0);
			if (node->getType() == 因子) {
				node = node->getChildren()->at(0);
				if (node->getType() == 標識符) {
					node = node->getChildren()->at(0);
					assert(node->getType() == 葉子節點);
					for (int i = table.size() - 1; i >= 0; i--) {
						if ((table[i]->isSameScope(TableItem::scope_i) || table[i]->isSameScope(0))
							&& table[i]->isSameName(&node->getWord()->getWord())) {
							return table[i]->getRetType() == CHAR;
						}
					}
				}
				else if (node->getType() == 字符) {
					return true;
				}
				else if (node->getType() == 有返回值函數調用語句) {
					node = node->getChildren()->at(0);
					assert(node->getType() == 標識符);
					node = node->getChildren()->at(0);
					assert(node->getType() == 葉子節點);
					for (int i = table.size() - 1; i >= 0; i--) {
						if (table[i]->isSameName(&node->getWord()->getWord())) {
							return table[i]->getRetType() == CHAR;
						}
					}
				}
			}
		}
	}
	return false;
}

/* Input: Word_of_標識符 in 因子, 賦值語句, 循環語句, 有/無返回值的函數調用語句, 讀語句
* Return: true when detected Error_C, false when not.
*/
bool TableTools::errorJudgerC(Word* word) {
	assert(word->getType() == IDENFR);
	for (int i = table.size() - 1; i >= 0; i--) {
		if ((table[i]->isSameScope(TableItem::scope_i) || table[i]->isSameScope(0)) 
			&& table[i]->isSameName(&word->getWord())) {
			return false;
		}
	}
	ErrorHandler::addErrorItem(ERROR_C, word->getLine());
	return true;
}

/* Input: Word_of_標識符 in 有/無返回值的函數調用語句, SymbolNode_of_值參數表
*Return: true when detected Error_D, false when not.
*/
bool TableTools::errorJudgerD(Word* word, SymbolNode* node) {
	int expectedParamNum = 0, actualParamNum = 0;
	for (int i = table.size() - 1; i >= 0; i--) {
		if ((table[i]->isSameScope(TableItem::scope_i) || table[i]->isSameScope(0))
			&& table[i]->isSameName(&word->getWord())) {
			expectedParamNum = table[i]->getParamNum();
			break;
		}
	}
	for (int i = 0; i < node->getChildren()->size(); i++) {
		if (node->getChildren()->at(i)->getType() == 表達式) {
			actualParamNum++;
		}
	}
	if (expectedParamNum == actualParamNum) {
		return false;
	}
	else {
		ErrorHandler::addErrorItem(ERROR_D, word->getLine());
		return true;
	}
}

/* Input: Word_of_標識符 in 有/無返回值的函數調用語句, SymbolNode_of_值參數表
*Return: true when detected Error_E, false when not.
*/
bool TableTools::errorJudgerE(Word* word, SymbolNode* node) {
	std::vector<int>* expectedParamRetType = nullptr;
	std::vector<int>* actualParamRetType = new std::vector<int>();
	for (int i = table.size() - 1; i >= 0; i--) {
		if ((table[i]->isSameScope(TableItem::scope_i) || table[i]->isSameScope(0))
			&& table[i]->isSameName(&word->getWord())) {
			expectedParamRetType = table[i]->getParamsRetType();
			break;
		}
	}
	for (int i = 0; i < node->getChildren()->size(); i++) {
		if (node->getChildren()->at(i)->getType() == 表達式) {
			actualParamRetType->push_back(isCharType(node->getChildren()->at(i)) ? CHAR : INT);
		}
	}
	assert(expectedParamRetType != nullptr);
	assert(expectedParamRetType->size() == actualParamRetType->size());
	for (int i = 0; i < expectedParamRetType->size(); i++) {
		if (actualParamRetType->at(i) != expectedParamRetType->at(i)) {
			ErrorHandler::addErrorItem(ERROR_E, word->getLine());
			return true;
		}
	}
	return false;
}

/* 
* stage = 1 : Mark the type of the function that is analyzed from parser.
* stage = 2 : Send the type of the return statement that got from parser.
* stage = 3 : Judge if the function is non-void with no return statement.
*/
bool TableTools::errorJudgerGH(int stage, int retType, int line) {
	static int funcType = -1;
	static int retCount = 0;
	if (stage == 1) {
		funcType = retType;
		retCount = 0;
	}
	if (stage == 2) {
		if (funcType == VOID) {
			if (retType != VOID) {
				ErrorHandler::addErrorItem(ERROR_G, line);
				return true;
			}
		}
		else if (funcType == INT) {
			retCount++;
			if (retType != INT) {
				ErrorHandler::addErrorItem(ERROR_H, line);
				return true;
			}
		}
		else if (funcType == CHAR) {
			retCount++;
			if (retType != CHAR) {
				ErrorHandler::addErrorItem(ERROR_H, line);
				return true;
			}
		}
	}
	if (stage == 3) {
		assert(funcType == INT || funcType == CHAR);
		if (retCount == 0) {
			ErrorHandler::addErrorItem(ERROR_H, line);
			return true;
		}
	}
	return false;
}

/* Input: Word_of_標識符 in 賦值語句
* Return: true when detected Error_J, false when not.
*/
bool TableTools::errorJudgerJ(Word* word) {
	assert(word->getType() == IDENFR);
	for (int i = table.size() - 1; i >= 0; i--) {
		if ((table[i]->isSameScope(TableItem::scope_i) || table[i]->isSameScope(0))
			&& table[i]->isSameName(&word->getWord())) {
			if (table[i]->getType() == CONST) {
				ErrorHandler::addErrorItem(ERROR_J, word->getLine());
				return true;
			}
			else {
				break;
			}
		}
	}
	return false;
}


bool TableTools::errorJudgerO(SymbolNode* node, int stage) {
	static bool declareTypeIsChar;
	if (stage == 1) {
		if (node->getType() == 類型標識符) {
			declareTypeIsChar = (node->getChildren()->at(0)->getWord()->getType() == CHARTK);
		} // In Var Definition
		else if (node->getType() == 表達式) {
			declareTypeIsChar = isCharType(node);
		} // In Switch Statement
		else {
			assert(false);
		}
	}
	else if (stage == 2) {
		assert(node->getType() == 常量);
		bool realTypeIsChar = (node->getChildren()->at(0)->getType() == 字符);
		if (declareTypeIsChar ^ realTypeIsChar) {
			while (node->getChildren()->size() != 0) {
				node = node->getChildren()->at(0);
			}
			ErrorHandler::addErrorItem(ERROR_O, node->getWord()->getLine());
			return true;
		}
	}
	return false;
}