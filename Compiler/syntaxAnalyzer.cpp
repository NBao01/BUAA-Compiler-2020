#include "syntaxAnalyzer.h"
#include "parser.h"
#include "symbols.h"
#include "tokens.h"
#include <fstream>

std::ofstream out("output.txt", std::ios_base::out);

void SyntaxAnalyzer::analyze() {
	getsym();
	absRoot = Parser::_程序();
}

void SyntaxAnalyzer::output() {
	outputdfs(absRoot);
}

void SyntaxAnalyzer::outputdfs(SymbolNode* node) {
	std::vector<SymbolNode*>::iterator it;
	for (it = node->getChildren()->begin(); it != node->getChildren()->end(); ++it) {
		outputdfs(*it);
	}
	if (node->getType() == 葉子節點) {
		out << tokens[node->getWord()->getType()] << " " << node->getWord()->getWord() << std::endl;
	}
	else if (
		node->getType() != 加法運算符	&&
		node->getType() != 乘法運算符	&&
		node->getType() != 關係運算符	&&
		node->getType() != 字符			&&
		node->getType() != 標識符		&&
		node->getType() != 類型標識符
		) {
		out << "<" << symbols[node->getType()] << ">" << std::endl;
	}
}