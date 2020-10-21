#include "lexicalAnalyzer.h"
#include "syntaxAnalyzer.h"
#include "error.h"

void doNothing() {
	return;
}

int main() {
	int step = 3;
	LexicalAnalyzer::analyze();
	(step == 1) ? LexicalAnalyzer::output() : doNothing();

	SyntaxAnalyzer::analyze();
	(step == 2) ? SyntaxAnalyzer::output() : doNothing();

	(step == 3) ? ErrorHandler::output() : doNothing();
	return 0;
}