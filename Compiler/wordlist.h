#ifndef __WORDLIST_H__
#define __WORDLIST_H__

#include <string>

class Wordlist {
private:
	// char * word;
	std::string& word;
	int type;
	int line;
public:
	Wordlist(const char* word, int type, int line);
	// char* getWord();
	std::string& getWord();
	int getType();
};

#endif // !__WORDLIST_H__
