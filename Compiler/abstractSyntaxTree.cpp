#include "abstractSyntaxTree.h"
#include "symbols.h"

SymbolNode* absRoot;

SymbolNode::SymbolNode(int type) {
	this->type = type;
	this->word = nullptr;
}

SymbolNode::SymbolNode(Word* word) {
	this->type = 葉子節點;
	this->word = word;
}

SymbolNode::SymbolNode(int type, Word* word) {
	this->type = type;
	this->addChild(new SymbolNode(word));
}

void SymbolNode::addChild(SymbolNode* node) {
	this->children.push_back(node);
}

int SymbolNode::getType() {
	return type;
}

Word* SymbolNode::getWord() {
	return word;
}
std::vector<SymbolNode*>* SymbolNode::getChildren() {
	return &children;
}