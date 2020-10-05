#include "parser.h"
#include "tokens.h"
#include "error.h"
#include "symbols.h"

std::vector<Word*> VoidFuncTable;

Word* word;
int it = 0;	// iterator of wordlist

void getsym() { 
	//static std::vector<Word*>::iterator it = wordlist.begin();
	if (it < wordlist.size()) {
		word = wordlist[it++];
	}
	else {
		//word = nullptr;
	}
}

Word* peeksym(int step) {
	Word* peeked = wordlist[it + step];
	return peeked;
}

// ＜程序＞ ::= ［＜常量说明＞］［＜变量说明＞］{＜有返回值函数定义＞|＜无返回值函数定义＞}＜主函数＞
SymbolNode* Parser::_程序() {
	SymbolNode* node = new SymbolNode(程序);
	if (word->getType() == CONSTTK) {
		node->addChild(_常量說明());
	}
	if ((word->getType() == INTTK || word->getType() == CHARTK) && peeksym(1)->getType() != LPARENT) {
		node->addChild(_變量說明());
	}
	while (word->getType() == INTTK || word->getType() == CHARTK || 
		(word->getType() == VOIDTK && peeksym(0)->getType() != MAINTK)) {
		if (word->getType() == INTTK || word->getType() == CHARTK) {
			node->addChild(_有返回值函數定義());
		}
		else if (word->getType() == VOIDTK) {
			node->addChild(_無返回值函數定義());
		}
	}
	node->addChild(_主函數());
	return node;
}

/* ＜常量说明＞ ::= const＜常量定义＞;{ const＜常量定义＞;} */
SymbolNode* Parser::_常量說明() {
	SymbolNode* node = new SymbolNode(常量說明);
	while (word->getType() == CONSTTK) {
		node->addChild(new SymbolNode(word));
		getsym();
		node->addChild(_常量定義());
		if (word->getType() == SEMICN) {
			node->addChild(new SymbolNode(word));
			getsym();
		}
		else error();
	}
	return node;
}

/* ＜常量定义＞ ::= int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞} | 
*                  char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}
*/
SymbolNode* Parser::_常量定義() {
	SymbolNode* node = new SymbolNode(常量定義);
	if (word->getType() == INTTK) {
		node->addChild(new SymbolNode(word));
		getsym();
		while (word->getType() == IDENFR) {
			node->addChild(_標識符());
			if (word->getType() == ASSIGN) {
				node->addChild(new SymbolNode(word));
				getsym();
			}
			else error();

			if (word->getType() == INTCON || word->getType() == PLUS || word->getType() == MINU) {
				node->addChild(_整數());
			}
			else error();

			if (word->getType() == COMMA) {
				node->addChild(new SymbolNode(word));
				getsym();
			}
			else {
				return node;
			}
		}
		error();
	}
	else if (word->getType() == CHARTK) {
		node->addChild(new SymbolNode(word));
		getsym();
		while (word->getType() == IDENFR) {
			node->addChild(_標識符());
			if (word->getType() == ASSIGN) {
				node->addChild(new SymbolNode(word));
				getsym();
			}
			else error();
			if (word->getType() == CHARCON) {
				node->addChild(_字符());
			}
			else error();
			if (word->getType() == COMMA) {
				node->addChild(new SymbolNode(word));
				getsym();
			}
			else {
				return node;
			}
		}
		error();
	}
	delete node;
	return nullptr;
}

/* ＜整数＞ ::= ［＋｜－］＜无符号整数＞ */
SymbolNode* Parser::_整數() {
	SymbolNode* node = new SymbolNode(整數);	
	if (word->getType() == PLUS || word->getType() == MINU) {
		node->addChild(new SymbolNode(word));	// word->type is PLUS or MINU
		getsym();
	}
	node->addChild(_無符號整數());
	return node;
}

/* ＜无符号整数＞ ::= ＜数字＞｛＜数字＞｝ */
SymbolNode* Parser::_無符號整數() {
	SymbolNode* node = new SymbolNode(無符號整數);
	node->addChild(new SymbolNode(word));	// word->type is INTCON
	getsym();
	return node;
}

/* ＜字符＞ ::= '＜加法运算符＞'｜'＜乘法运算符＞'｜'＜字母＞'｜'＜数字＞' */
SymbolNode* Parser::_字符() {
	SymbolNode* node = new SymbolNode(字符);
	node->addChild(new SymbolNode(word));	// word->type is CHARCON
	getsym();
	return node;
}

/* ＜变量说明＞ ::= ＜变量定义＞;{＜变量定义＞;} */
SymbolNode* Parser::_變量說明() {
	SymbolNode* node = new SymbolNode(變量説明);
	while ((word->getType() == INTTK || word->getType() == CHARTK) && peeksym(1)->getType() != LPARENT) {
		node->addChild(_變量定義());
		if (word->getType() == SEMICN) {
			node->addChild(new SymbolNode(word));
			getsym();
		}
		else error();
	}
	return node;
}

/* ＜变量定义＞ ::= ＜变量定义无初始化＞|＜变量定义及初始化＞ */
SymbolNode* Parser::_變量定義() {
	SymbolNode* node = new SymbolNode(變量定義);
	int peeknum = 0;
	while (peeksym(peeknum)->getType() != ASSIGN && peeksym(peeknum)->getType() != COMMA && peeksym(peeknum)->getType() != SEMICN) {
		peeknum++;
	}
	if (peeksym(peeknum)->getType() == COMMA || peeksym(peeknum)->getType() == SEMICN) {
		node->addChild(_變量定義無初始化());
	}
	else if (peeksym(peeknum)->getType() == ASSIGN) {
		node->addChild(_變量定義及初始化());
	}
	else error();
	return node;
}

/* 
* ＜变量定义无初始化＞ ::= ＜类型标识符＞
	(＜标识符＞|＜标识符＞'['＜无符号整数＞']'|＜标识符＞'['＜无符号整数＞']''['＜无符号整数＞']')
	{,(＜标识符＞|＜标识符＞'['＜无符号整数＞']'|＜标识符＞'['＜无符号整数＞']''['＜无符号整数＞']' )}
*/
SymbolNode* Parser::_變量定義無初始化() {
	SymbolNode* node = new SymbolNode(變量定義無初始化);
	node->addChild(_類型標識符());
	while (word->getType() == IDENFR) {
		node->addChild(_標識符());
		if (word->getType() == LBRACK) {
			node->addChild(new SymbolNode(word));
			getsym();
			node->addChild(_無符號整數());
			getsym();
			node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
			getsym();
			if (word->getType() == LBRACK) {
				node->addChild(new SymbolNode(word));
				getsym();
				node->addChild(_無符號整數());
				getsym();
				node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
				getsym();
			}
		}
		if (word->getType() == COMMA) {
			node->addChild(new SymbolNode(word));
			getsym();
		}
		else return node;
	}
	error();
	delete node;
	return nullptr;
}

/* 
* ＜变量定义及初始化＞ ::=
	＜类型标识符＞＜标识符＞=＜常量＞|
	＜类型标识符＞＜标识符＞'['＜无符号整数＞']'='{'＜常量＞{,＜常量＞}'}'|
	＜类型标识符＞＜标识符＞'['＜无符号整数＞']''['＜无符号整数＞']'=
						'{''{'＜常量＞{,＜常量＞}'}'{, '{'＜常量＞{,＜常量＞}'}'}'}'
*/
SymbolNode* Parser::_變量定義及初始化() {
	SymbolNode* node = new SymbolNode(變量定義及初始化);
	node->addChild(_類型標識符());
	node->addChild(_標識符());
	if (word->getType() == LBRACK) {
		node->addChild(new SymbolNode(word));
		getsym();
		node->addChild(_無符號整數());
		getsym();
		node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
		getsym();
		if (word->getType() == LBRACK) {
			// ＜标识符＞'['＜无符号整数＞']''['＜无符号整数＞']'='{''{'＜常量＞{ ,＜常量＞ }'}'{, '{'＜常量＞{ ,＜常量＞ }'}'}'}'
			node->addChild(new SymbolNode(word));
			getsym();
			node->addChild(_無符號整數());
			getsym();
			node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
			getsym();
			node->addChild(new SymbolNode(word));	// word->getType() is ASSIGN
			getsym();
			node->addChild(new SymbolNode(word));	// word->getType() is LBRACE
			getsym();
			node->addChild(new SymbolNode(word));	// word->getType() is LBRACE
			getsym();
			node->addChild(_常量());
			while (word->getType() == COMMA) {
				getsym();
				node->addChild(_常量());
			}
			node->addChild(new SymbolNode(word));	// word->getType() is RBRACE
			getsym();
			if (word->getType() == COMMA) {
				getsym();
				node->addChild(new SymbolNode(word));	// word->getType() is LBRACE
				getsym();
				node->addChild(_常量());
				while (word->getType() == COMMA) {
					getsym();
					node->addChild(_常量());
				}
				node->addChild(new SymbolNode(word));	// word->getType() is RBRACE
				getsym();
				node->addChild(new SymbolNode(word));	// word->getType() is RBRACE
				getsym();
			}
		} 
		else { // ＜标识符＞'['＜无符号整数＞']'='{'＜常量＞{,＜常量＞}'}'
			node->addChild(new SymbolNode(word));	// word->getType() is ASSIGN
			getsym();
			node->addChild(new SymbolNode(word));	// word->getType() is LBRACE
			getsym();
			node->addChild(_常量());
			while (word->getType() == COMMA) {
				getsym();
				node->addChild(_常量());
			}
			node->addChild(new SymbolNode(word));	// word->getType() is RBRACE
			getsym();
		}
	}
	else { // ＜标识符＞=＜常量＞
		node->addChild(new SymbolNode(word));	// word->getType() is ASSIGN
		getsym();
		node->addChild(_常量());
	}
	return node;
}

SymbolNode* Parser::_標識符() {
	SymbolNode* node = new SymbolNode(標識符);
	if (word->getType() == IDENFR) {
		node->addChild(new SymbolNode(word));
		getsym();
	}
	else error();
	return node;
}

// ＜类型标识符＞ ::= int | char 
SymbolNode* Parser::_類型標識符() {
	SymbolNode* node = new SymbolNode(類型標識符);
	if (word->getType() == INTTK || word->getType() == CHARTK) {
		node->addChild(new SymbolNode(word));
		getsym();
	}
	else error();
	return node;
}

// ＜常量＞ ::= ＜整数＞|＜字符＞
SymbolNode* Parser::_常量() {
	SymbolNode* node = new SymbolNode(常量);
	if (word->getType() == INTCON) {
		node->addChild(_整數());
		getsym();
	}
	else if (word->getType() == CHARCON) {
		node->addChild(_字符());
		getsym();
	}
	else error();
	return node;
}

// ＜有返回值函数定义＞ ::= ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}'
SymbolNode* Parser::_有返回值函數定義() {
	SymbolNode* node = new SymbolNode(有返回值函數定義);
	node->addChild(_聲明頭部());
	node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
	getsym();
	node->addChild(_參數表());
	node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
	getsym();
	node->addChild(new SymbolNode(word));	// word->getType() is LBRACE
	getsym();
	node->addChild(_複合語句());
	node->addChild(new SymbolNode(word));	// word->getType() is RBRACE
	getsym();
	return node;
}

// ＜无返回值函数定义＞ ::= void＜标识符＞'('＜参数表＞')''{'＜复合语句＞'}'
SymbolNode* Parser::_無返回值函數定義() {
	SymbolNode* node = new SymbolNode(無返回值函數定義);
	node->addChild(new SymbolNode(word));	// word->getType() is VOIDTK
	getsym();
	VoidFuncTable.push_back(word);			// For the JUDGEMENT of the call of void/non-void function
	node->addChild(_標識符());
	node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
	getsym();
	node->addChild(_參數表());
	node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
	getsym();
	node->addChild(new SymbolNode(word));	// word->getType() is LBRACE
	getsym();
	node->addChild(_複合語句());
	node->addChild(new SymbolNode(word));	// word->getType() is RBRACE
	getsym();
	return node;
}

// ＜声明头部＞ ::=  int＜标识符＞ |char＜标识符＞
SymbolNode* Parser::_聲明頭部() {
	SymbolNode* node = new SymbolNode(聲明頭部);
	node->addChild(new SymbolNode(word));	// word->getType() is INTTK or CHARTK
	getsym();
	node->addChild(_標識符());
	return node;
}

// ＜参数表＞ ::= ＜类型标识符＞＜标识符＞{,＜类型标识符＞＜标识符＞}| ＜空＞
SymbolNode* Parser::_參數表() {
	SymbolNode* node = new SymbolNode(參數表);
	while (word->getType() == INTTK || word->getType() == CHARTK) {
		node->addChild(_類型標識符());
		node->addChild(_標識符());
		if (word->getType() == COMMA) {
			node->addChild(new SymbolNode(word));
			getsym();
		}
	}
	return node;
}

// ＜复合语句＞ ::= ［＜常量说明＞］［＜变量说明＞］＜语句列＞
SymbolNode* Parser::_複合語句() {
	SymbolNode* node = new SymbolNode(複合語句);
	if (word->getType() == CONSTTK) {
		node->addChild(_常量說明());
	}
	if (word->getType() == INTTK || word->getType() == CHARTK) {
		node->addChild(_變量說明());
	}
	node->addChild(_語句列());
	return node;
}

// ＜语句列＞ ::= ｛＜语句＞｝
SymbolNode* Parser::_語句列() {
	SymbolNode* node = new SymbolNode(語句列);
	while (word->getType() != RBRACE) {
		node->addChild(_語句());
	}
	return node;
}

/* ＜语句＞ ::= ＜循环语句＞｜＜条件语句＞| ＜有返回值函数调用语句＞;  |＜无返回值函数调用语句＞;｜
*		＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜情况语句＞｜＜空＞;|＜返回语句＞; | '{'＜语句列＞'}'
*/
SymbolNode* Parser::_語句() {
	SymbolNode* node = new SymbolNode(語句);
	if (word->getType() == WHILETK || word->getType() == FORTK) {
		node->addChild(_循環語句());
	}
	else if (word->getType() == IFTK) {
		node->addChild(_條件語句());
	}
	else if (word->getType() == IDENFR) {
		int peeknum = 0;
		while (peeksym(peeknum)->getType() != ASSIGN && peeksym(peeknum)->getType() != LPARENT) {
			peeknum++;
		}
		if (peeksym(peeknum)->getType() == ASSIGN) {
			node->addChild(_賦值語句());
		}
		else if (peeksym(peeknum)->getType() == LPARENT) {
			bool isVoidFunc = false;
			for (int i = 0; i < VoidFuncTable.size(); i++) {
				if (word->getWord() == VoidFuncTable[i]->getWord()) {
					isVoidFunc = true;
					break;
				}
			}
			if (isVoidFunc) {
				node->addChild(_無返回值函數調用語句());
			}
			else {
				node->addChild(_有返回值函數調用語句());
			}
		}
		node->addChild(new SymbolNode(word));	// word->getType() is SEMICN
		getsym();
	}
	else if (word->getType() == SCANFTK) {
		node->addChild(_讀語句());
		node->addChild(new SymbolNode(word));	// word->getType() is SEMICN
		getsym();
	}
	else if (word->getType() == PRINTFTK) {
		node->addChild(_寫語句());
		node->addChild(new SymbolNode(word));	// word->getType() is SEMICN
		getsym();
	}
	else if (word->getType() == SWITCHTK) {
		node->addChild(_情況語句());
	}
	else if (word->getType() == SEMICN) {
		node->addChild(new SymbolNode(word));	// word->getType() is SEMICN
		getsym();
	}
	else if (word->getType() == RETURNTK) {
		node->addChild(_返回語句());
		node->addChild(new SymbolNode(word));	// word->getType() is SEMICN
		getsym();
	}
	else if (word->getType() == LBRACE) {
		node->addChild(new SymbolNode(word));	// word->getType() is LBRACE
		getsym();
		node->addChild(_語句列());
		node->addChild(new SymbolNode(word));	// word->getType() is RBRACE
		getsym();
	}
	//else if ()  ? ＜有返回值函数调用语句＞;  |＜无返回值函数调用语句＞;
	// ＜赋值语句＞;
	return node;
}

/* ＜循环语句＞ ::=  while '('＜条件＞')'＜语句＞| 
*		for'('＜标识符＞＝＜表达式＞;＜条件＞;＜标识符＞＝＜标识符＞(+|-)＜步长＞')'＜语句＞
*/
SymbolNode* Parser::_循環語句() {
	SymbolNode* node = new SymbolNode(循環語句);
	if (word->getType() == WHILETK) {
		node->addChild(new SymbolNode(word));
		getsym();
		node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
		getsym();
		node->addChild(_條件());
		node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
		getsym();
		node->addChild(_語句());
	}
	else if (word->getType() == FORTK) {
		node->addChild(new SymbolNode(word));
		getsym();
		node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
		getsym();
		node->addChild(_標識符());
		node->addChild(new SymbolNode(word));	// word->getType() is ASSIGN
		getsym();
		node->addChild(_表達式());
		node->addChild(new SymbolNode(word));	// word->getType() is SEMICN
		getsym();
		node->addChild(_條件());
		node->addChild(new SymbolNode(word));	// word->getType() is SEMICN
		getsym();
		node->addChild(_標識符());
		node->addChild(new SymbolNode(word));	// word->getType() is ASSIGN
		getsym();
		node->addChild(_標識符());
		node->addChild(new SymbolNode(word));	// word->getType() is PLUS or MINU
		getsym();
		node->addChild(_步長());
		node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
		getsym();
		node->addChild(_語句());
	}
	return node;
}

// ＜条件语句＞ :: = if '('＜条件＞')'＜语句＞［else＜语句＞］
SymbolNode* Parser::_條件語句() {
	SymbolNode* node = new SymbolNode(條件語句);
	node->addChild(new SymbolNode(word));	// word->getType() is IFTK
	node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
	getsym();
	node->addChild(_條件());
	node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
	getsym();
	node->addChild(_語句());
	if (word->getType() == ELSETK) {
		node->addChild(new SymbolNode(word));	// word->getType() is ELSETK
		node->addChild(_語句());
	}
	return node;
}

// ＜条件＞ ::= ＜表达式＞＜关系运算符＞＜表达式＞
SymbolNode* Parser::_條件() {
	SymbolNode* node = new SymbolNode(條件);
	node->addChild(_表達式());
	node->addChild(_關係運算符());
	node->addChild(_表達式());
	return node;
}

// ＜表达式＞ ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}
SymbolNode* Parser::_表達式() {
	SymbolNode* node = new SymbolNode(表達式);
	if (word->getType() == PLUS || word->getType() == MINU) {
		node->addChild(new SymbolNode(word));	// word->getType() is PLUS or MINU
		getsym();
	}
	node->addChild(_項());
	while (word->getType() == PLUS || word->getType() == MINU) {
		node->addChild(_加法運算符());
		node->addChild(_項());
	}
	return node;
}

// ＜步长＞::= ＜无符号整数＞  
SymbolNode* Parser::_步長() {
	SymbolNode* node = new SymbolNode(步長);
	node->addChild(_無符號整數());
	return node;
}

// ＜关系运算符＞ ::=  <｜<=｜>｜>=｜!=｜==
SymbolNode* Parser::_關係運算符() {
	SymbolNode* node = new SymbolNode(關係運算符);
	if (word->getType() == LSS || word->getType() == LEQ || word->getType() == GRE ||
		word->getType() == GEQ || word->getType() == NEQ || word->getType() == EQL) {
		node->addChild(new SymbolNode(word));
		getsym();
	}
	else error();
	return node;
}

// ＜项＞ ::= ＜因子＞{＜乘法运算符＞＜因子＞}
SymbolNode* Parser::_項() {
	SymbolNode* node = new SymbolNode(項);
	node->addChild(_因子());
	while (word->getType() == MULT || word->getType() == DIV) {
		node->addChild(_乘法運算符());
		node->addChild(_因子());
	}
	return node;
}

// ＜加法运算符＞ ::= +｜-
SymbolNode* Parser::_加法運算符() {
	SymbolNode* node = new SymbolNode(加法運算符);
	if (word->getType() == PLUS || word->getType() == MINU) {
		node->addChild(new SymbolNode(word));
		getsym();
	}
	else error();
	return node;
}

/* ＜因子＞ ::= ＜标识符＞｜＜标识符＞'['＜表达式＞']'|
*				＜标识符＞'['＜表达式＞']''['＜表达式＞']'|'('＜表达式＞')'｜
*				＜整数＞|＜字符＞｜＜有返回值函数调用语句＞
*/
SymbolNode* Parser::_因子() {
	SymbolNode* node = new SymbolNode(因子);
	if (word->getType() == IDENFR && peeksym(0)->getType() != LPARENT) {
		node->addChild(_標識符());
		if (word->getType() == LBRACK) {
			node->addChild(new SymbolNode(word));	// word->getType() is LBRACK
			getsym();
			node->addChild(_表達式());
			node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
			getsym();
			if (word->getType() == LBRACK) {
				node->addChild(new SymbolNode(word));	// word->getType() is LBRACK
				getsym();
				node->addChild(_表達式());
				node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
				getsym();
			}
		}
	}
	else if (word->getType() == LPARENT) {
		node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
		getsym();
		node->addChild(_表達式());
		node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
		getsym();
	}
	else if (word->getType() == INTCON) {
		node->addChild(_整數());
	}
	else if (word->getType() == CHARCON) {
		node->addChild(_字符());
	}
	else if (word->getType() == IDENFR) {
		node->addChild(_有返回值函數調用語句());
	}
	return node;
}

// ＜乘法运算符＞ ::= *｜/
SymbolNode* Parser::_乘法運算符() {
	SymbolNode* node = new SymbolNode(乘法運算符);
	if (word->getType() == MULT || word->getType() == DIV) {
		node->addChild(new SymbolNode(word));
		getsym();
	}
	else error();
	return node;
}

// ＜有返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')' 
SymbolNode* Parser::_有返回值函數調用語句() {
	SymbolNode* node = new SymbolNode(有返回值函數調用語句);
	node->addChild(_標識符());
	node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
	getsym();
	node->addChild(_值參數表());
	node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
	getsym();
	return node;
}

// ＜值参数表＞ ::= ＜表达式＞{,＜表达式＞}｜＜空＞
SymbolNode* Parser::_值參數表() {
	SymbolNode* node = new SymbolNode(值參數表);
	if (word->getType() != RPARENT) {
		node->addChild(_表達式());
		while (word->getType() == COMMA) {
			node->addChild(new SymbolNode(word));	// word->getType() is COMMA
			getsym();
			node->addChild(_表達式());
		}
	}
	return node;
}

// scanf '('＜标识符＞')'
SymbolNode* Parser::_讀語句() {
	SymbolNode* node = new SymbolNode(讀語句);
	node->addChild(new SymbolNode(word));	// word->getType() is SCANFTK
	getsym();
	node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
	getsym();
	node->addChild(_標識符());
	node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
	getsym();
	return node;
}

// ＜写语句＞ ::= printf '(' ＜字符串＞,＜表达式＞ ')'| printf '('＜字符串＞ ')'| printf '('＜表达式＞')' 
SymbolNode* Parser::_寫語句() {
	SymbolNode* node = new SymbolNode(寫語句);
	node->addChild(new SymbolNode(word));	// word->getType() is PRINTFTK
	getsym();
	node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
	getsym();
	if (word->getType() == STRCON) {
		node->addChild(_字符串());
		if (word->getType() == COMMA) {
			node->addChild(new SymbolNode(word));	// word->getType() is COMMA
			getsym();
			node->addChild(_表達式());
		}
	}
	else {
		node->addChild(_表達式());
	}
	node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
	getsym();

	return node;
}

// ＜字符串＞ ::= "｛十进制编码为32,33,35-126的ASCII字符｝" 
SymbolNode* Parser::_字符串() {
	SymbolNode* node = new SymbolNode(字符串);
	if (word->getType() == STRCON) {
		node->addChild(new SymbolNode(word));
		getsym();
	}
	else error();
	return node;
}

// ＜返回语句＞ ::= return['('＜表达式＞')']   
SymbolNode* Parser::_返回語句() {
	SymbolNode* node = new SymbolNode(返回語句);
	if (word->getType() == RETURNTK) {
		node->addChild(new SymbolNode(word));
		getsym();
		if (word->getType() == LPARENT) {
			node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
			getsym();
			node->addChild(_表達式());
			node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
			getsym();
		}
	}
	else error();
	return node;
}

// ＜情况语句＞ ::= switch ‘(’＜表达式＞‘)’ ‘{’＜情况表＞＜缺省＞‘}’
SymbolNode* Parser::_情況語句() {
	SymbolNode* node = new SymbolNode(情況語句);
	node->addChild(new SymbolNode(word));	// word->getType() is SWITCHTK
	getsym();
	node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
	getsym();
	node->addChild(_表達式());
	node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
	getsym();
	node->addChild(new SymbolNode(word));	// word->getType() is LBRACE
	getsym();
	node->addChild(_情況表());
	node->addChild(_缺省());
	node->addChild(new SymbolNode(word));	// word->getType() is RBRACE
	getsym();
	return node;
}

// ＜情况表＞ ::= ＜情况子语句＞{＜情况子语句＞}
SymbolNode* Parser::_情況表() {
	SymbolNode* node = new SymbolNode(情況表);
	while (word->getType() == CASETK) {
		node->addChild(_情況子語句());
	}
	return node;
}

// ＜缺省＞ ::= default :＜语句＞
SymbolNode* Parser::_缺省() {
	SymbolNode* node = new SymbolNode(缺省);
	node->addChild(new SymbolNode(word));	// word->getType() is DEFAULTTK
	getsym();
	node->addChild(new SymbolNode(word));	// word->getType() is COLON
	getsym();
	node->addChild(_語句());
	return node;
}

//＜情况子语句＞ :: = case＜常量＞：＜语句＞
SymbolNode* Parser::_情況子語句() {
	SymbolNode* node = new SymbolNode(情況子語句);
	node->addChild(new SymbolNode(word));	// word->getType() is CASETK
	getsym();
	node->addChild(_常量());
	node->addChild(new SymbolNode(word));	// word->getType() is COLON
	getsym();
	node->addChild(_語句());
	return node;
}

/* ＜赋值语句＞ ::= ＜标识符＞＝＜表达式＞|
*					＜标识符＞'['＜表达式＞']'=＜表达式＞|
*					＜标识符＞'['＜表达式＞']''['＜表达式＞']' =＜表达式＞
*/
SymbolNode* Parser::_賦值語句() {
	SymbolNode* node = new SymbolNode(賦值語句);
	node->addChild(_標識符());
	if (word->getType() == LBRACK) {
		node->addChild(new SymbolNode(word));	// word->getType() is LBRACK
		getsym();
		node->addChild(_表達式());
		node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
		getsym();
		if (word->getType() == LBRACK) {
			node->addChild(new SymbolNode(word));	// word->getType() is LBRACK
			getsym();
			node->addChild(_表達式());
			node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
			getsym();
		}
	}
	node->addChild(new SymbolNode(word));	// word->getType() is ASSIGN
	getsym();
	node->addChild(_表達式());
	return node;
}

// ＜无返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')'
SymbolNode* Parser::_無返回值函數調用語句() {
	SymbolNode* node = new SymbolNode(無返回值函數調用語句);
	node->addChild(_標識符());
	node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
	getsym();
	node->addChild(_值參數表());
	node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
	getsym();
	return node;
}

// ＜主函数＞ ::= void main‘(’‘)’ ‘{’＜复合语句＞‘}’
SymbolNode* Parser::_主函數() {
	SymbolNode* node = new SymbolNode(主函數);
	node->addChild(new SymbolNode(word));	// word->getType() is VOIDTK
	getsym();
	node->addChild(new SymbolNode(word));	// word->getType() is MAINTK
	getsym();
	node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
	getsym();
	node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
	getsym();
	node->addChild(new SymbolNode(word));	// word->getType() is LBRACE
	getsym();
	node->addChild(_複合語句());
	node->addChild(new SymbolNode(word));	// word->getType() is RBRACE
	getsym();
	return node;
}