#ifndef __ABSTRACTSYNTAXTREE_H__
#define __ABSTRACTSYNTAXTREE_H__

#include "wordlist.h"
#include <vector>

class SymbolNode {
private:
	int type;
	Word* word;
	std::vector<SymbolNode*> children;
public:
	SymbolNode(int type);
	SymbolNode(Word* word);
	SymbolNode(int type, Word* word);
	void addChild(SymbolNode* node);
	int getType();
	Word* getWord();
	std::vector<SymbolNode*>* getChildren();
};

extern SymbolNode* absRoot;

#endif // !__ABSTRACTSYNTAXTREE_H__

