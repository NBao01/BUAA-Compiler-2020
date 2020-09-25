#ifndef __LEXICALANALYZER_H__
#define __LEXICALANALYZER_H__

#include <vector>
#include "wordlist.h"

class LexicalAnalyzer {
private:
	static std::vector<Wordlist *> wordlist;
public:
	static void analyze();
	static void output();
};

#endif // !__LEXICALANALYZER_H__