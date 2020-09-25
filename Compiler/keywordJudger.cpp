#include "keywordJudger.h"
#include "tokens.h"

const std::string KeywordJudger::dict[15] = {
	"case",
	"char",
	"const",
	"default",
	"else",
	"for",
	"if",
	"int",
	"main",
	"printf",
	"return",
	"scanf",
	"switch",
	"void",
	"while"
}; // Ascending Order for Binary Search

const int types[15] = {
	CASETK, CHARTK, CONSTTK, DEFAULTTK, ELSETK, FORTK, IFTK, INTTK,
	MAINTK, PRINTFTK, RETURNTK, SCANFTK, SWITCHTK, VOIDTK, WHILETK
};

/* By binary search the keyword list, find out if the word is a keyword.
* return the type if the word is a keyword.
* return IDENFR if the word is not a keyword.
*/
int KeywordJudger::isKeyword(std::string& word) {
	// TODO: tolower the string.
	std::string s("");
	for (int i = 0; i < word.size(); i++) {
		s += tolower(word[i]);
	}
	int lo = 0, hi = 14;
	int mid;
	while (lo <= hi) {
		mid = (lo + hi) / 2;
		if (dict[mid] == s) {
			return types[mid];
		}
		else if (dict[mid] < s) {
			lo = mid + 1;
		}
		else if (dict[mid] > s) {
			hi = mid - 1;
		}
	}
	return IDENFR;
}