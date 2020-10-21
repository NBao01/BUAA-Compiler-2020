#include "wordlist.h"

std::vector<Word*> wordlist;

Word::Word(std::string* word, int type, int line) : word(*word) {
	this->type = type;
	this->line = line;
}

std::string& Word::getWord() {
	return word;
}

int Word::getType() {
	return type;
}

int Word::getLine() {
	return line;
}