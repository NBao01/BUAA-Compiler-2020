#ifndef __KEYWORDJUDGER_H__
#define __KEYWORDJUDGER_H__

#include <string>

class KeywordJudger {
private:
	const static std::string dict[15];
	const static int tokens[15];
public:
	static int isKeyword(char* word);
};

#endif // !__KEYWORDJUDGER_H__