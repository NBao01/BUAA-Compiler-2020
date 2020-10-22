#include "table.h"
#include <cassert>
#include "symbols.h"
#include "tokens.h"
#include <iostream>

std::vector<TableItem*> table;

int TableItem::scope_i = 0;

std::string* TableTools::name = nullptr;
int TableTools::type = -1;
int TableTools::retType = -1;

TableItem::TableItem(std::string* name, int type, int retType) {
	this->name = name;
	this->type = type;
	this->retType = retType;
	scope = scope_i;
	dimension = 0;
	paramNum = 0;
	params = nullptr;
}

TableItem* TableItem::newConstTableItem(std::string* name, int type, int retType) {
	assert(type == CONST && retType != VOID);
	return new TableItem(name, type, retType);
}

TableItem* TableItem::newVarTableItem(std::string* name, int type, int retType, int dimension) {
	assert(type == VAR && retType != VOID);
	TableItem* ti = new TableItem(name, type, retType);
	ti->setDimension(dimension);
	return ti;
}

TableItem* TableItem::newFuncTableItem(std::string* name, int type, int retType, int paramNum, std::vector<int>* params) {
	assert(type == FUNC);
	TableItem* ti = new TableItem(name, type, retType);
	ti->setParams(paramNum, params);
	scope_i++;
	return ti;
}

void TableItem::setDimension(int dimension) {
	this->dimension = dimension;
}

void TableItem::setParams(int paramNum, std::vector<int>* params) {
	this->paramNum = paramNum;
	this->params = params;
}

void TableTools::addConsts(SymbolNode* node) {
	type = CONST;
	addConstsDfs(node);
}

void TableTools::addConstsDfs(SymbolNode* node) {
	std::vector<SymbolNode*>::iterator it;
	for (it = node->getChildren()->begin(); it != node->getChildren()->end(); ++it) {
		addConsts(*it);
	}
	if (node->getType() == 葉子節點) {
		// std::cout << tokens[node->getWord()->getType()] << " " << node->getWord()->getWord() << std::endl;
		if (node->getWord()->getType() == INTTK) {
			retType = INT;
		}
		else if (node->getWord()->getType() == CHARTK) {
			retType = CHAR;
		}

		if (node->getWord()->getType() == IDENFR) {
			name = &node->getWord()->getWord();
			table.push_back(TableItem::newConstTableItem(name, type, retType));
		}
	}
}