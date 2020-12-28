#include "lexicalAnalyzer.h"
#include "syntaxAnalyzer.h"
#include "error.h"
#include "ir.h"
#include "mips.h"
#include "irOptimizer.h"

void doNothing() {
	return;
}

int main() {
	int step = 5;
	bool optimize = true;

	LexicalAnalyzer::analyze();
	(step == 1) ? LexicalAnalyzer::output() : doNothing();

	SyntaxAnalyzer::analyze();
	(step == 2) ? SyntaxAnalyzer::output() : doNothing();

	if (ErrorHandler::hasError()) {
		ErrorHandler::output();
	}
	else {
		(optimize) ? IrOptimizer::optimize() : doNothing();

		(step == 4 || step == 5) ? IrGenerator::output() : doNothing();

		MipsGenerator::generate();
		(step == 5) ? MipsGenerator::output() : doNothing();
	}

	return 0;
}