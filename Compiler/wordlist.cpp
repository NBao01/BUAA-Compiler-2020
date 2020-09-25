#include "wordlist.h"

Wordlist::Wordlist(std::string* word, int type, int line) : word(*word) {
	this->type = type;
	this->line = line;
}

std::string& Wordlist::getWord() {
	return word;
}

int Wordlist::getType() {
	return type;
}