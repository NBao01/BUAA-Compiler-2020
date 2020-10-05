#include "lexicalAnalyzer.h"
#include "syntaxAnalyzer.h"

void doNothing() {
	return;
}

int main() {
	int step = 2;
	LexicalAnalyzer::analyze();
	(step == 1) ? LexicalAnalyzer::output() : doNothing();

	SyntaxAnalyzer::analyze();
	(step == 2) ? SyntaxAnalyzer::output() : doNothing();
	return 0;
}