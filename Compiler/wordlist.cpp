#include "wordlist.h"

Wordlist::Wordlist(const char* word, int type, int line) : word(*(new std::string(word))) {
	this->type = type;
	this->line = line;
}

std::string& Wordlist::getWord() {
	return word;
}

int Wordlist::getType() {
	return type;
}