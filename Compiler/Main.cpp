#include "lexicalAnalyzer.h"
#include "syntaxAnalyzer.h"
#include "error.h"
#include "ir.h"

void doNothing() {
	return;
}

int main() {
	int step = 4;
	LexicalAnalyzer::analyze();
	(step == 1) ? LexicalAnalyzer::output() : doNothing();

	SyntaxAnalyzer::analyze();
	(step == 2) ? SyntaxAnalyzer::output() : doNothing();

	(step == 3) ? ErrorHandler::output() : doNothing();

	(step == 4) ? IrGenerator::output() : doNothing();
	return 0;
}