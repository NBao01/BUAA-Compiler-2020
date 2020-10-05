#ifndef __SYNTAXANALYZER_H__
#define __SYNTAXANALYZER_H__

#include "abstractSyntaxTree.h"

class SyntaxAnalyzer {
private:
	static void outputdfs(SymbolNode* node);
public:
	static void analyze();
	static void output();
};

#endif // !__SYNTAXANALYZER_H__
