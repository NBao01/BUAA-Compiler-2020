#include "table.h"
#include <cassert>
#include "symbols.h"
#include "tokens.h"
#include <iostream>
#include "error.h"
#include <sstream>

std::vector<TableItem*> table;

int TableItem::scope_i = 0;
int TableItem::offset_i = 0;

int TableTools::it_prev= 0;
std::string* TableTools::name = nullptr;
int TableTools::type = -1;
int TableTools::retType = -1;
std::vector<int> TableTools::stackSpace;

TableItem::TableItem(std::string* name, int type, int retType, int scope) {
	this->name = name;
	this->type = type;
	this->retType = retType;
	this->scope = scope;
	// These below will be set by setter
	dimension = 0;
	dims[0] = 0;
	dims[1] = 0;
	paramNum = 0;
	paramsRetType = nullptr;
	this->label = nullptr;
	this->offset = 0;
	this->initialValue = 0;
	this->initialValues = nullptr;
	this->cache = 0;
}

TableItem* TableItem::newConstTableItem(std::string* name, int type, int retType, int initialValue) {
	assert(type == CONST && retType != VOID);
	TableItem* ti = new TableItem(name, type, retType, scope_i);

	// Make Label for Global Const
	if (scope_i == 0) {
		std::string* label = new std::string();
		if (retType == INT) {
			*label += "global_const_int_";
		}
		else if (retType == CHAR) {
			*label += "global_const_char_";
		}
		*label += *name;
		ti->setLabel(label);
	}
	else {
		ti->setOffset(offset_i);
		offset_i += 4;
	}

	ti->setInitialValue(initialValue);

	return ti;
}

TableItem* TableItem::newVarTableItem(std::string* name, int type, int retType, 
	int dimension, int dim0, int dim1, int initialValue, std::vector<int>* initialValues) {
	assert(type == VAR && retType != VOID);
	TableItem* ti = new TableItem(name, type, retType, scope_i);

	ti->setDimension(dimension, dim0, dim1);

	// Make Label for Global Var
	if (scope_i == 0) {
		std::string* label = new std::string();
		if (retType == INT) {
			*label += "global_var_int_";
		}
		else if (retType == CHAR) {
			*label += "global_var_char_";
		}
		*label += *name;
		ti->setLabel(label);
	}
	else {
		ti->setOffset(offset_i);
		if (dimension == 0) {
			offset_i += 4;
		}
		else if(dimension == 1) {
			offset_i += 4 * dim0;
		}
		else if (dimension == 2) {
			offset_i += 4 * dim0 * dim1;
		}
	}

	ti->setInitialValue (initialValue );
	ti->setInitialValues(initialValues);

	return ti;
}

TableItem* TableItem::newFuncTableItem(std::string* name, int type, int retType) {
	assert(type == FUNC);
	TableItem* ti = new TableItem(name, type, retType, 0); // The Function Definition has scope 0
	//ti->setParams(paramNum, params);
	TableTools::setStackSpaceOfScope(offset_i);	// set stack space for each scope.
	scope_i++;
	offset_i = 0;
	return ti;
}

TableItem* TableItem::newParamTableItem(std::string* name, int type, int retType) {
	assert(type == PARAM);
	TableItem * ti = new TableItem(name, type, retType, scope_i);
	ti->setOffset(offset_i);
	offset_i += 4;	// offset of params by stack pointer
	return ti;
}

void TableItem::setDimension(int dimension, int dim0, int dim1) {
	this->dimension = dimension;
	this->dims[0] = dim0;
	this->dims[1] = dim1;
}

void TableItem::setParams(int paramNum, std::vector<int>* paramsRetType) {
	this->paramNum = paramNum;
	this->paramsRetType = paramsRetType;
}

void TableItem::setLabel(std::string* label) {
	this->label = label;
}

void TableItem::setOffset(int offset) {
	this->offset = offset;
}

void TableItem::setInitialValue(int initialValue) {
	this->initialValue = initialValue;
}

void TableItem::setInitialValues(std::vector<int>* initialValues) {
	this->initialValues = initialValues;
}

void TableItem::setCache(int reg) {
	this->cache = reg;
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

std::string* TableItem::getName() {
	return name;
}

int TableItem::getType() {
	return type;
}

int TableItem::getRetType() {
	return retType;
}

int TableItem::getScope() {
	return scope;
}

int TableItem::getDimension() {
	return dimension;
}

int TableItem::getDim0() {
	return dims[0];
}

int TableItem::getDim1() {
	return dims[1];
}

int TableItem::getParamNum() {
	return paramNum;
}

std::vector<int>* TableItem::getParamsRetType() {
	return paramsRetType;
}

std::string* TableItem::getLabel() {
	return label;
}

int TableItem::getOffset() {
	return offset;
}

int TableItem::getInitialValue() {
	return initialValue;
}

std::vector<int>* TableItem::getInitialValues() {
	return initialValues;
}

int TableItem::getCache() {
	return cache;
}

void TableTools::setStackSpaceOfScope(int space) {
	stackSpace.push_back(space);
}

int TableTools::getstackSpaceOfScope(int scope) {
	return stackSpace.at(scope);
}

// parse initial value for consts and vars. return ascii if charcon, return intVal if intcon, plus, minus.
int TableTools::parseInitialValue(int& i) {
	assert(wordlist[i]->getType() == INTCON || wordlist[i]->getType() == CHARCON || 
		wordlist[i]->getType() == PLUS || wordlist[i]->getType() == MINU);

	int value = 0;

	if (wordlist[i]->getType() == INTCON) {
		std::stringstream ss; ss << wordlist[i]->getWord(); ss >> value;
	}
	else if (wordlist[i]->getType() == CHARCON) {
		value = (int)wordlist[i]->getWord().at(0);	// ASCII
	}
	else if (wordlist[i]->getType() == PLUS) {
		std::stringstream ss; ss << wordlist[++i]->getWord(); ss >> value;
	}
	else if (wordlist[i]->getType() == MINU) {
		std::stringstream ss; ss << wordlist[++i]->getWord(); ss >> value;
		value = -value;
	}
	// Don't need a 'i++' because it will ++ in for loop of addConsts and addVars.
	return value;
}

// Add consts to the table, add error_b to errorlist if there is any redefined consts
void TableTools::addConsts(int it) {
	type = CONST;
	int initialValue = 0;
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

			// Get the Initial Value of the const
			assert(wordlist[++i]->getType() == ASSIGN);
			initialValue = parseInitialValue(++i);

			table.push_back(TableItem::newConstTableItem(name, type, retType, initialValue));
		}
	}
	it_prev = it;
}

// Add variables to the table, add error_b to errorlist if there is any redefined variables
void TableTools::addVars(int it) {
	type = VAR;
	int dimension = 0;
	int dim0 = 1, dim1 = 1;	// Initialize to 1, so that 'dim0 * dim1 = num_of_array' no matter what the dimension is.
	int initialValue = 0;
	std::vector<int>* initialValues = nullptr;
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

		// Get the Dimension of the variable
		if (wordlist[i]->getType() == LBRACK) {
			dimension++;
			assert(wordlist[++i]->getType() == INTCON);
			std::stringstream ss; ss << wordlist[i]->getWord(); ss >> dim0;
			assert(wordlist[++i]->getType() == RBRACK);

			if (wordlist[++i]->getType() == LBRACK) {
				dimension++;
				assert(wordlist[++i]->getType() == INTCON);
				std::stringstream ss; ss << wordlist[i]->getWord(); ss >> dim1;
				assert(wordlist[++i]->getType() == RBRACK);
			}
		}

		if (wordlist[i]->getType() == ASSIGN) {
			i++;
			if (wordlist[i]->getType() == LBRACE) {
				// Array Variable
				initialValues = new std::vector<int>();
				i++;
				while (wordlist[i]->getType() != RBRACE) {
					if (wordlist[i]->getType() == LBRACE) {
						while (wordlist[i]->getType() != RBRACE) {
							if (wordlist[i]->getType() == INTCON || wordlist[i]->getType() == CHARCON ||
								wordlist[i]->getType() == PLUS || wordlist[i]->getType() == MINU) {
								initialValues->push_back(parseInitialValue(i)); // Two Dimension Array
							}
							i++;
						}
					}
					else if (wordlist[i]->getType() == INTCON || wordlist[i]->getType() == CHARCON ||
						wordlist[i]->getType() == PLUS || wordlist[i]->getType() == MINU) {
						initialValues->push_back(parseInitialValue(i)); // One Dimension Array
					}
					i++;
				}
			}
			else {
				// Simple Variable
				initialValue = parseInitialValue(i);
			}
		}

		if (wordlist[i]->getType() == COMMA || wordlist[i]->getType() == SEMICN) {
			table.push_back(TableItem::newVarTableItem(name, type, retType, dimension, dim0, dim1, initialValue, initialValues));
			dimension = 0; dim0 = 1; dim1 = 1;
			initialValue = 0;
			initialValues = nullptr;
		}
	}
	assert(initialValues == nullptr);
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
			if (wordlist[i]->getType() == IDENFR || wordlist[i]->getType() == MAINTK) {
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

TableItem* TableTools::search(std::string* name) {
	for (int i = table.size() - 1; i >= 0; i--) {
		if ((table[i]->isSameScope(TableItem::scope_i) || table[i]->isSameScope(0)) &&
			table[i]->isSameName(name)) {
			return table[i];
		}
	}
	return nullptr;
}

void TableTools::search(std::string* str, int* type, std::string** label) {
	for (int i = table.size() - 1; i >= 0; i--) {
		if ((table[i]->isSameScope(TableItem::scope_i) || table[i]->isSameScope(0)) &&
			table[i]->isSameName(str)) {
			*type = table[i]->getRetType();
			if (table[i]->getScope() == 0) {
				*label = new std::string("global_");
			}
			else {
				std::stringstream ss;
				std::string scope_num;
				ss << table[i]->getScope();
				ss >> scope_num;
				*label = new std::string("scope_" + scope_num + "_");
			}

			if (table[i]->getType() == CONST) {
				**label += "const_";
			}
			else if (table[i]->getType() == VAR) {
				**label += "var_";
			}

			if (table[i]->getRetType() == INT) {
				**label += "int_";
			}
			else if (table[i]->getRetType() == CHAR) {
				**label += "char_";
			}

			**label += *str;
			return;
		}
	}
}