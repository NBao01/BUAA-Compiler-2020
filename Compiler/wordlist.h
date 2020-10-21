#ifndef __WORDLIST_H__
#define __WORDLIST_H__

#include <string>
#include <vector>

class Word {
private:
	// char * word;
	std::string& word;
	int type;
	int line;
public:
	Word(std::string* word, int type, int line);
	// char* getWord();
	std::string& getWord();
	int getType();
	int getLine();
};

extern std::vector<Word*> wordlist;

#endif // !__WORDLIST_H__
