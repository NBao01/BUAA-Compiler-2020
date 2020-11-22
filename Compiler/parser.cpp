#include "parser.h"
#include "tokens.h"
#include "error.h"
#include "symbols.h"
#include "table.h"
#include <sstream>
#include "ir.h"
#include "irDefinitions.h"
#include <cassert>
#include "mips.h"

Word* word;
Word* prevWord;
int it = 0;	// iterator of wordlist

void getsym() { 
	//static std::vector<Word*>::iterator it = wordlist.begin();
	if (it < wordlist.size()) {
		if (it > 0) {
			prevWord = word;
		}
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
	TableTools::setStackSpaceOfScope(TableItem::offset_i);	// set stack space for main scope.
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
			node->addChild(new SymbolNode(word));	// word->getType() is SEMICN
			getsym();
		}
		else {
			// ERROR_K JUDGER
			ErrorHandler::addErrorItem(ERROR_K, prevWord->getLine());
			// ERROR_K JUDGER END
		}
	}
	TableTools::addConsts(it - 1);
	return node;
}

/* ＜常量定义＞ ::= int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞} | 
*                  char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}
*/
SymbolNode* Parser::_常量定義() {
	int type = 0, num = 0;
	std::string* str = nullptr;
	SymbolNode* node = new SymbolNode(常量定義);
	if (word->getType() == INTTK) {
		node->addChild(new SymbolNode(word));
		getsym();
		while (word->getType() == IDENFR) {
			node->addChild(_標識符(&type, &str));
			if (word->getType() == ASSIGN) {
				node->addChild(new SymbolNode(word));
				getsym();
			}
			else error();

			if (word->getType() == INTCON || word->getType() == PLUS || word->getType() == MINU) {
				node->addChild(_整數(&num));
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
			node->addChild(_標識符(&type, &str));
			if (word->getType() == ASSIGN) {
				node->addChild(new SymbolNode(word));
				getsym();
			}
			else error();
			if (word->getType() == CHARCON) {
				node->addChild(_字符(&num));
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

/* ＜整数＞ ::= ［＋｜－］＜无符号整数＞ */
SymbolNode* Parser::_整數(int* num) {
	SymbolNode* node = new SymbolNode(整數);
	int sign = 1;
	if (word->getType() == PLUS || word->getType() == MINU) {
		if (word->getType() == MINU) {
			sign = -1;
		}
		node->addChild(new SymbolNode(word));	// word->type is PLUS or MINU
		getsym();
	}
	node->addChild(_無符號整數(num));
	*num = sign * (*num);
	return node;
}

/* ＜无符号整数＞ ::= ＜数字＞｛＜数字＞｝ */
SymbolNode* Parser::_無符號整數() {
	SymbolNode* node = new SymbolNode(無符號整數);
	node->addChild(new SymbolNode(word));	// word->type is INTCON
	getsym();
	return node;
}

/* ＜无符号整数＞ ::= ＜数字＞｛＜数字＞｝ */
SymbolNode* Parser::_無符號整數(int* num) {
	SymbolNode* node = new SymbolNode(無符號整數);
	std::stringstream ss(word->getWord());
	ss >> *num;
	node->addChild(new SymbolNode(word));	// word->type is INTCON
	getsym();
	return node;
}

/* ＜字符＞ ::= '＜加法运算符＞'｜'＜乘法运算符＞'｜'＜字母＞'｜'＜数字＞' */
SymbolNode* Parser::_字符() {
	SymbolNode* node = new SymbolNode(字符);

	// ERROR_A JUDGER
	if (word->getWord().size() == 0) {
		ErrorHandler::addErrorItem(ERROR_A, word->getLine());
	} // NO CHAR ERROR
	else {
		char c = word->getWord().at(0);
		if (!(isalpha(c) || isdigit(c) ||
			c == '+' || c == '-' || c == '*' || c == '/' || c == '_')) {
			ErrorHandler::addErrorItem(ERROR_A, word->getLine());
		} // ILLEGAL CHAR ERROR
	}
	// ERROR_A JUDGER END

	node->addChild(new SymbolNode(word));	// word->type is CHARCON
	getsym();
	return node;
}

/* ＜字符＞ ::= '＜加法运算符＞'｜'＜乘法运算符＞'｜'＜字母＞'｜'＜数字＞' */
SymbolNode* Parser::_字符(std::string** str) {
	SymbolNode* node = new SymbolNode(字符);

	// ERROR_A JUDGER
	if (word->getWord().size() == 0) {
		ErrorHandler::addErrorItem(ERROR_A, word->getLine());
	} // NO CHAR ERROR
	else {
		char c = word->getWord().at(0);
		if (!(isalpha(c) || isdigit(c) ||
			c == '+' || c == '-' || c == '*' || c == '/' || c == '_')) {
			ErrorHandler::addErrorItem(ERROR_A, word->getLine());
		} // ILLEGAL CHAR ERROR
	}
	// ERROR_A JUDGER END

	*str = &word->getWord();
	node->addChild(new SymbolNode(word));	// word->type is CHARCON
	getsym();
	return node;
}

/* ＜字符＞ ::= '＜加法运算符＞'｜'＜乘法运算符＞'｜'＜字母＞'｜'＜数字＞' */
SymbolNode* Parser::_字符(int* ascii) {
	SymbolNode* node = new SymbolNode(字符);

	// ERROR_A JUDGER
	if (word->getWord().size() == 0) {
		ErrorHandler::addErrorItem(ERROR_A, word->getLine());
	} // NO CHAR ERROR
	else {
		char c = word->getWord().at(0);
		if (!(isalpha(c) || isdigit(c) ||
			c == '+' || c == '-' || c == '*' || c == '/' || c == '_')) {
			ErrorHandler::addErrorItem(ERROR_A, word->getLine());
		} // ILLEGAL CHAR ERROR
	}
	// ERROR_A JUDGER END

	*ascii = (int)word->getWord().at(0);
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
			node->addChild(new SymbolNode(word));	// word->getType() is SEMICN
			getsym();
		}
		else {
			// ERROR_K JUDGER
			ErrorHandler::addErrorItem(ERROR_K, prevWord->getLine());
			// ERROR_K JUDGER END
		}
	}
	TableTools::addVars(it - 1);
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
	int type;
	std::string* str;

	SymbolNode* node = new SymbolNode(變量定義無初始化);
	node->addChild(_類型標識符());
	while (word->getType() == IDENFR) {
		node->addChild(_標識符(&type, &str));
		if (word->getType() == LBRACK) {
			node->addChild(new SymbolNode(word));
			getsym();
			node->addChild(_無符號整數());
			if (word->getType() == RBRACK) {
				node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
				getsym();
			}
			else {
				// ERROR_M JUDGER
				ErrorHandler::addErrorItem(ERROR_M, prevWord->getLine());
				// ERROR_M JUDGER END
			}
			if (word->getType() == LBRACK) {
				node->addChild(new SymbolNode(word));
				getsym();
				node->addChild(_無符號整數());
				if (word->getType() == RBRACK) {
					node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
					getsym();
				}
				else {
					// ERROR_M JUDGER
					ErrorHandler::addErrorItem(ERROR_M, prevWord->getLine());
					// ERROR_M JUDGER END
				}
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
	int type, num;
	std::string* str;

	SymbolNode* nodeForErrorO;
	int numInDim1 = 0, numInDim2 = 0;	// The num of the elements in the array, can have two dimensions.
	int counter1 = 0, counter2 = 0;		// Counter of the num of the elements.

	SymbolNode* node = new SymbolNode(變量定義及初始化);
	node->addChild((nodeForErrorO = _類型標識符()));

	// ERROR_O JUDGER STAGE 1 
	TableTools::errorJudgerO(nodeForErrorO, 1);
	// ERROR_O JUDGER STAGE 1 END

	node->addChild(_標識符(&type, &str));
	if (word->getType() == LBRACK) {
		node->addChild(new SymbolNode(word));
		getsym();

		/* ERROR_N Related */std::stringstream ss(word->getWord()); ss >> numInDim1;

		node->addChild(_無符號整數());
		if (word->getType() == RBRACK) {
			node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
			getsym();
		}
		else {
			// ERROR_M JUDGER
			ErrorHandler::addErrorItem(ERROR_M, prevWord->getLine());
			// ERROR_M JUDGER END
		}
		if (word->getType() == LBRACK) {
			// ＜标识符＞'['＜无符号整数＞']''['＜无符号整数＞']'='{''{'＜常量＞{ ,＜常量＞ }'}'{, '{'＜常量＞{ ,＜常量＞ }'}'}'}'
			node->addChild(new SymbolNode(word));
			getsym();

			/* ERROR_N Related */std::stringstream ss(word->getWord()); ss >> numInDim2;

			node->addChild(_無符號整數());
			if (word->getType() == RBRACK) {
				node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
				getsym();
			}
			else {
				// ERROR_M JUDGER
				ErrorHandler::addErrorItem(ERROR_M, prevWord->getLine());
				// ERROR_M JUDGER END
			}
			node->addChild(new SymbolNode(word));	// word->getType() is ASSIGN
			getsym();
			node->addChild(new SymbolNode(word));	// word->getType() is LBRACE
			getsym();
			node->addChild(new SymbolNode(word));	// word->getType() is LBRACE
			getsym();
			node->addChild((nodeForErrorO = _常量()));

			/* ERROR_N Related */counter2++;

			// ERROR_O JUDGER STAGE 2 
			TableTools::errorJudgerO(nodeForErrorO, 2);
			// ERROR_O JUDGER STAGE 2 END

			while (word->getType() == COMMA) {
				node->addChild(new SymbolNode(word));	// word->getType() is COMMA
				getsym();
				node->addChild((nodeForErrorO = _常量()));

				/* ERROR_N Related */counter2++;

				// ERROR_O JUDGER STAGE 2 
				TableTools::errorJudgerO(nodeForErrorO, 2);
				// ERROR_O JUDGER STAGE 2 END

			}
			node->addChild(new SymbolNode(word));	// word->getType() is RBRACE

			// ERROR_N JUDGER
			if (counter2 != numInDim2) {
				ErrorHandler::addErrorItem(ERROR_N, word->getLine());
			}
			counter1++;
			counter2 = 0;
			// ERROR_N JUDGER END

			getsym();
			while (word->getType() == COMMA) {
				node->addChild(new SymbolNode(word));	// word->getType() is COMMA
				getsym();
				node->addChild(new SymbolNode(word));	// word->getType() is LBRACE
				getsym();
				node->addChild((nodeForErrorO = _常量()));

				/* ERROR_N Related */counter2++;

				// ERROR_O JUDGER STAGE 2 
				TableTools::errorJudgerO(nodeForErrorO, 2);
				// ERROR_O JUDGER STAGE 2 END

				while (word->getType() == COMMA) {
					node->addChild(new SymbolNode(word));	// word->getType() is COMMA
					getsym();
					node->addChild((nodeForErrorO = _常量()));

					/* ERROR_N Related */counter2++;

					// ERROR_O JUDGER STAGE 2 
					TableTools::errorJudgerO(nodeForErrorO, 2);
					// ERROR_O JUDGER STAGE 2 END

				}
				node->addChild(new SymbolNode(word));	// word->getType() is RBRACE

				// ERROR_N JUDGER
				if (counter2 != numInDim2) {
					ErrorHandler::addErrorItem(ERROR_N, word->getLine());
				}
				counter1++;
				counter2 = 0;
				// ERROR_N JUDGER END

				getsym();
			}
			node->addChild(new SymbolNode(word));	// word->getType() is RBRACE

			// ERROR_N JUDGER
			if (counter1 != numInDim1) {
				ErrorHandler::addErrorItem(ERROR_N, word->getLine());
			}
			// ERROR_N JUDGER END

			getsym();
		} 
		else { // ＜标识符＞'['＜无符号整数＞']'='{'＜常量＞{,＜常量＞}'}'
			node->addChild(new SymbolNode(word));	// word->getType() is ASSIGN
			getsym();
			node->addChild(new SymbolNode(word));	// word->getType() is LBRACE
			getsym();
			node->addChild((nodeForErrorO = _常量()));
			/* ERROR_N Related */counter1++;
			// ERROR_O JUDGER STAGE 2 
			TableTools::errorJudgerO(nodeForErrorO, 2);
			// ERROR_O JUDGER STAGE 2 END
			while (word->getType() == COMMA) {
				node->addChild(new SymbolNode(word));	// word->getType() is COMMA
				getsym();
				node->addChild((nodeForErrorO = _常量()));
				/* ERROR_N Related */counter1++;
				// ERROR_O JUDGER STAGE 2 
				TableTools::errorJudgerO(nodeForErrorO, 2);
				// ERROR_O JUDGER STAGE 2 END
			}
			node->addChild(new SymbolNode(word));	// word->getType() is RBRACE

			// ERROR_N JUDGER
			if (counter1 != numInDim1) {
				ErrorHandler::addErrorItem(ERROR_N, word->getLine());
			}
			// ERROR_N JUDGER END

			getsym();
		}
	}
	else { // ＜标识符＞=＜常量＞
		node->addChild(new SymbolNode(word));	// word->getType() is ASSIGN
		getsym();
		node->addChild((nodeForErrorO = _常量(&num)));

		// ERROR_O JUDGER STAGE 2 
		TableTools::errorJudgerO(nodeForErrorO, 2);
		// ERROR_O JUDGER STAGE 2 END
	}
	return node;
}


SymbolNode* Parser::_標識符(int* type, std::string** str) {
	SymbolNode* node = new SymbolNode(標識符);
	*type = IDTYPE;
	*str = &word->getWord();
	if (word->getType() == IDENFR) {
		node->addChild(new SymbolNode(word));
		getsym();
	}
	else error();
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
	if (word->getType() == INTCON || word->getType() == PLUS || word->getType() == MINU) {
		node->addChild(_整數());
	}
	else if (word->getType() == CHARCON) {
		node->addChild(_字符());
	}
	else error();
	return node;
}

// ＜常量＞ ::= ＜整数＞|＜字符＞
SymbolNode* Parser::_常量(int *num) {
	SymbolNode* node = new SymbolNode(常量);
	if (word->getType() == INTCON || word->getType() == PLUS || word->getType() == MINU) {
		node->addChild(_整數(num));
	}
	else if (word->getType() == CHARCON) {
		node->addChild(_字符(num));
	}
	else error();
	return node;
}

// ＜有返回值函数定义＞ ::= ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}'
SymbolNode* Parser::_有返回值函數定義() {

	// ERROR_GH JUDGER STAGE 1 
	if (word->getType() == INTTK) {
		TableTools::errorJudgerGH(1, INT);
	}
	else if (word->getType() == CHARTK) {
		TableTools::errorJudgerGH(1, CHAR);
	}
	// ERROR_GH JUDGER STAGE 1 END

	SymbolNode* node = new SymbolNode(有返回值函數定義);
	node->addChild(_聲明頭部());
	node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
	getsym();
	node->addChild(_參數表());
	if (word->getType() == RPARENT) {
		node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
		getsym();
	}
	else {
		// ERROR_L JUDGER
		ErrorHandler::addErrorItem(ERROR_L, prevWord->getLine());
		// ERROR_L JUDGER END
	}
	node->addChild(new SymbolNode(word));	// word->getType() is LBRACE
	getsym();
	TableTools::addFunc(it - 1);
	node->addChild(_複合語句());

	// ERROR_GH JUDGER STAGE 3
	TableTools::errorJudgerGH(3, 0, word->getLine());
	// ERROR_GH JUDGER STAGE 3 END

	node->addChild(new SymbolNode(word));	// word->getType() is RBRACE
	getsym();
	return node;
}

// ＜无返回值函数定义＞ ::= void＜标识符＞'('＜参数表＞')''{'＜复合语句＞'}'
SymbolNode* Parser::_無返回值函數定義() {

	// ERROR_GH JUDGER STAGE 1 
	TableTools::errorJudgerGH(1, VOID);
	// ERROR_GH JUDGER STAGE 1 END

	SymbolNode* node = new SymbolNode(無返回值函數定義);
	node->addChild(new SymbolNode(word));	// word->getType() is VOIDTK
	getsym();
	node->addChild(_標識符());
	node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
	getsym();
	node->addChild(_參數表());
	if (word->getType() == RPARENT) {
		node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
		getsym();
	}
	else {
		// ERROR_L JUDGER
		ErrorHandler::addErrorItem(ERROR_L, prevWord->getLine());
		// ERROR_L JUDGER END
	}
	node->addChild(new SymbolNode(word));	// word->getType() is LBRACE
	getsym();
	TableTools::addFunc(it - 1);
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
		/*int peeknum = 0;
		while (peeksym(peeknum)->getType() != ASSIGN && peeksym(peeknum)->getType() != LPARENT) {
			peeknum++;
		}*/
		if (peeksym(0)->getType() != LPARENT) {
			node->addChild(_賦值語句());
		}
		else {
			if (TableTools::isVoidFunc(&word->getWord())) {
				node->addChild(_無返回值函數調用語句());
			}
			else {
				node->addChild(_有返回值函數調用語句());
			}
		}
		if (word->getType() == SEMICN) {
			node->addChild(new SymbolNode(word));	// word->getType() is SEMICN
			getsym();
		}
		else {
			// ERROR_K JUDGER
			ErrorHandler::addErrorItem(ERROR_K, prevWord->getLine());
			// ERROR_K JUDGER END
		}
	}
	else if (word->getType() == SCANFTK) {
		node->addChild(_讀語句());
		if (word->getType() == SEMICN) {
			node->addChild(new SymbolNode(word));	// word->getType() is SEMICN
			getsym();
		}
		else {
			// ERROR_K JUDGER
			ErrorHandler::addErrorItem(ERROR_K, prevWord->getLine());
			// ERROR_K JUDGER END
		}
	}
	else if (word->getType() == PRINTFTK) {
		node->addChild(_寫語句());
		if (word->getType() == SEMICN) {
			node->addChild(new SymbolNode(word));	// word->getType() is SEMICN
			getsym();
		}
		else {
			// ERROR_K JUDGER
			ErrorHandler::addErrorItem(ERROR_K, prevWord->getLine());
			// ERROR_K JUDGER END
		}
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
		if (word->getType() == SEMICN) {
			node->addChild(new SymbolNode(word));	// word->getType() is SEMICN
			getsym();
		}
		else {
			// ERROR_K JUDGER
			ErrorHandler::addErrorItem(ERROR_K, prevWord->getLine());
			// ERROR_K JUDGER END
		}
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
		if (word->getType() == RPARENT) {
			node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
			getsym();
		}
		else {
			// ERROR_L JUDGER
			ErrorHandler::addErrorItem(ERROR_L, prevWord->getLine());
			// ERROR_L JUDGER END
		}
		node->addChild(_語句());
	}
	else if (word->getType() == FORTK) {
		node->addChild(new SymbolNode(word));
		getsym();
		node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
		getsym();

		// ERROR_C JUDGER
		TableTools::errorJudgerC(word);
		// ERROR_C JUDGER END

		// ERROR_J JUDGER
		TableTools::errorJudgerJ(word);
		// ERROR_J JUDGER END

		node->addChild(_標識符());
		node->addChild(new SymbolNode(word));	// word->getType() is ASSIGN
		getsym();
		node->addChild(_表達式());
		if (word->getType() == SEMICN) {
			node->addChild(new SymbolNode(word));	// word->getType() is SEMICN
			getsym();
		}
		else {
			// ERROR_K JUDGER
			ErrorHandler::addErrorItem(ERROR_K, prevWord->getLine());
			// ERROR_K JUDGER END
		}
		node->addChild(_條件());
		if (word->getType() == SEMICN) {
			node->addChild(new SymbolNode(word));	// word->getType() is SEMICN
			getsym();
		}
		else {
			// ERROR_K JUDGER
			ErrorHandler::addErrorItem(ERROR_K, prevWord->getLine());
			// ERROR_K JUDGER END
		}
		node->addChild(_標識符());
		node->addChild(new SymbolNode(word));	// word->getType() is ASSIGN
		getsym();
		node->addChild(_標識符());
		node->addChild(new SymbolNode(word));	// word->getType() is PLUS or MINU
		getsym();
		node->addChild(_步長());
		if (word->getType() == RPARENT) {
			node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
			getsym();
		}
		else {
			// ERROR_L JUDGER
			ErrorHandler::addErrorItem(ERROR_L, prevWord->getLine());
			// ERROR_L JUDGER END
		}
		node->addChild(_語句());
	}
	return node;
}

// ＜条件语句＞ :: = if '('＜条件＞')'＜语句＞［else＜语句＞］
SymbolNode* Parser::_條件語句() {
	SymbolNode* node = new SymbolNode(條件語句);
	node->addChild(new SymbolNode(word));	// word->getType() is IFTK
	getsym();
	node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
	getsym();
	node->addChild(_條件());
	if (word->getType() == RPARENT) {
		node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
		getsym();
	}
	else {
		// ERROR_L JUDGER
		ErrorHandler::addErrorItem(ERROR_L, prevWord->getLine());
		// ERROR_L JUDGER END
	}
	node->addChild(_語句());
	if (word->getType() == ELSETK) {
		node->addChild(new SymbolNode(word));	// word->getType() is ELSETK
		getsym();
		node->addChild(_語句());
	}
	return node;
}

// ＜条件＞ ::= ＜表达式＞＜关系运算符＞＜表达式＞
SymbolNode* Parser::_條件() {
	int line; SymbolNode* nodeForErrorF;
	SymbolNode* node = new SymbolNode(條件);

	// ERROR_F JUDGER
	line = word->getLine();
	node->addChild((nodeForErrorF = _表達式()));
	if (TableTools::isCharType(nodeForErrorF)) {
		ErrorHandler::addErrorItem(ERROR_F, line);
	}
	// ERROR_F JUDGER END

	node->addChild(_關係運算符());

	// ERROR_F JUDGER
	line = word->getLine();
	node->addChild((nodeForErrorF = _表達式()));
	if (TableTools::isCharType(nodeForErrorF)) {
		ErrorHandler::addErrorItem(ERROR_F, line);
	}
	// ERROR_F JUDGER END

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

// ＜表达式＞ ::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}
SymbolNode* Parser::_表達式(int* type, int* num, std::string** str) {
	int _type = 0, _num = 0;
	std::string* _str = nullptr;
	SymbolNode* node = new SymbolNode(表達式);
	bool minu = false;
	if (word->getType() == PLUS || word->getType() == MINU) {
		if (word->getType() == MINU) {
			minu = true;
		}
		node->addChild(new SymbolNode(word));	// word->getType() is PLUS or MINU
		getsym();
	}
	node->addChild(_項(type, num, str));
	if (minu) {
		*str = IrGenerator::addNormalIr(IR_SUB, INTTYPE, *type, 0, *num, nullptr, *str);
		*type = TMPTYPE;
		*num = 0;
	}
	while (word->getType() == PLUS || word->getType() == MINU) {
		if (word->getType() == PLUS) {
			node->addChild(_加法運算符());
			node->addChild(_項(&_type, &_num, &_str));
			*str = IrGenerator::addNormalIr(IR_ADD, *type, _type, *num, _num, *str, _str);
			*type = TMPTYPE;
			*num = 0;
		}
		if (word->getType() == MINU) {
			node->addChild(_加法運算符());
			node->addChild(_項(&_type, &_num, &_str));
			*str = IrGenerator::addNormalIr(IR_SUB, *type, _type, *num, _num, *str, _str);
			*type = TMPTYPE;
			*num = 0;
		}
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

// ＜项＞ ::= ＜因子＞{＜乘法运算符＞＜因子＞}
SymbolNode* Parser::_項(int* type, int* num, std::string** str) {
	int _type = 0, _num = 0;
	std::string* _str = nullptr;
	SymbolNode* node = new SymbolNode(項);
	node->addChild(_因子(type, num, str));
	while (word->getType() == MULT || word->getType() == DIV) {
		if (word->getType() == MULT) {
			node->addChild(_乘法運算符());
			node->addChild(_因子(&_type, &_num, &_str));
			*str = IrGenerator::addNormalIr(IR_MUL, *type, _type, *num, _num, *str, _str);
			*type = TMPTYPE;
			*num = 0;
		}
		else if (word->getType() == DIV) {
			node->addChild(_乘法運算符());
			node->addChild(_因子(&_type, &_num, &_str));
			*str = IrGenerator::addNormalIr(IR_DIV, *type, _type, *num, _num, *str, _str);
			*type = TMPTYPE;
			*num = 0;
		}
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

		// ERROR_C JUDGER
		TableTools::errorJudgerC(word);
		// ERROR_C JUDGER END

		node->addChild(_標識符());
		int line; SymbolNode* nodeForErrorI;
		if (word->getType() == LBRACK) {
			node->addChild(new SymbolNode(word));	// word->getType() is LBRACK
			getsym();
			// ERROR_I JUDGER
			line = word->getLine();
			node->addChild((nodeForErrorI = _表達式()));
			if (TableTools::isCharType(nodeForErrorI)) {
				ErrorHandler::addErrorItem(ERROR_I, line);
			}
			// ERROR_I JUDGER END
			if (word->getType() == RBRACK) {
				node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
				getsym();
			}
			else {
				// ERROR_M JUDGER
				ErrorHandler::addErrorItem(ERROR_M, prevWord->getLine());
				// ERROR_M JUDGER END
			}
			if (word->getType() == LBRACK) {
				node->addChild(new SymbolNode(word));	// word->getType() is LBRACK
				getsym();
				// ERROR_I JUDGER
				line = word->getLine();
				node->addChild((nodeForErrorI = _表達式()));
				if (TableTools::isCharType(nodeForErrorI)) {
					ErrorHandler::addErrorItem(ERROR_I, line);
				}
				// ERROR_I JUDGER END
				if (word->getType() == RBRACK) {
					node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
					getsym();
				}
				else {
					// ERROR_M JUDGER
					ErrorHandler::addErrorItem(ERROR_M, prevWord->getLine());
					// ERROR_M JUDGER END
				}
			}
		}
	}
	else if (word->getType() == LPARENT) {
		node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
		getsym();
		node->addChild(_表達式());
		if (word->getType() == RPARENT) {
			node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
			getsym();
		}
		else {
			// ERROR_L JUDGER
			ErrorHandler::addErrorItem(ERROR_L, prevWord->getLine());
			// ERROR_L JUDGER END
		}
	}
	else if (word->getType() == INTCON || word->getType() == PLUS || word->getType() == MINU) {
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

/* ＜因子＞ ::= ＜标识符＞｜＜标识符＞'['＜表达式＞']'|
*				＜标识符＞'['＜表达式＞']''['＜表达式＞']'|'('＜表达式＞')'｜
*				＜整数＞|＜字符＞｜＜有返回值函数调用语句＞
*/
SymbolNode* Parser::_因子(int* type, int* num, std::string** str) {
	SymbolNode* node = new SymbolNode(因子);
	if (word->getType() == IDENFR && peeksym(0)->getType() != LPARENT) {

		// ERROR_C JUDGER
		TableTools::errorJudgerC(word);
		// ERROR_C JUDGER END

		node->addChild(_標識符(type, str));
		int line; SymbolNode* nodeForErrorI;
		if (word->getType() == LBRACK) {
			node->addChild(new SymbolNode(word));	// word->getType() is LBRACK
			getsym();
			// ERROR_I JUDGER
			line = word->getLine();
			node->addChild((nodeForErrorI = _表達式()));
			if (TableTools::isCharType(nodeForErrorI)) {
				ErrorHandler::addErrorItem(ERROR_I, line);
			}
			// ERROR_I JUDGER END
			if (word->getType() == RBRACK) {
				node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
				getsym();
			}
			else {
				// ERROR_M JUDGER
				ErrorHandler::addErrorItem(ERROR_M, prevWord->getLine());
				// ERROR_M JUDGER END
			}
			if (word->getType() == LBRACK) {
				node->addChild(new SymbolNode(word));	// word->getType() is LBRACK
				getsym();
				// ERROR_I JUDGER
				line = word->getLine();
				node->addChild((nodeForErrorI = _表達式()));
				if (TableTools::isCharType(nodeForErrorI)) {
					ErrorHandler::addErrorItem(ERROR_I, line);
				}
				// ERROR_I JUDGER END
				if (word->getType() == RBRACK) {
					node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
					getsym();
				}
				else {
					// ERROR_M JUDGER
					ErrorHandler::addErrorItem(ERROR_M, prevWord->getLine());
					// ERROR_M JUDGER END
				}
			}
		}
	}
	else if (word->getType() == LPARENT) {
		node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
		getsym();
		node->addChild(_表達式(type, num, str));
		if (word->getType() == RPARENT) {
			node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
			getsym();
		}
		else {
			// ERROR_L JUDGER
			ErrorHandler::addErrorItem(ERROR_L, prevWord->getLine());
			// ERROR_L JUDGER END
		}
	}
	else if (word->getType() == INTCON || word->getType() == PLUS || word->getType() == MINU) {
		*type = INTTYPE;
		node->addChild(_整數(num));
	}
	else if (word->getType() == CHARCON) {
		*type = CHTYPE;
		node->addChild(_字符(str));
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
	Word* wordForErrorDE = word;
	SymbolNode* nodeForErrorDE;
	SymbolNode* node = new SymbolNode(有返回值函數調用語句);

	// ERROR_C JUDGER
	bool errorC = TableTools::errorJudgerC(word);
	// ERROR_C JUDGER END

	node->addChild(_標識符());
	node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
	getsym();
	node->addChild((nodeForErrorDE = _值參數表()));		//node->addChild(_值參數表());
	if (word->getType() == RPARENT) {
		node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
		getsym();
	}
	else {
		// ERROR_L JUDGER
		ErrorHandler::addErrorItem(ERROR_L, prevWord->getLine());
		// ERROR_L JUDGER END
	}

	// ERROR_D JUDGER
	bool errorD = errorC ? true : TableTools::errorJudgerD(wordForErrorDE, nodeForErrorDE);
	// ERROR_D JUDGER END

	// ERROR_E JUDGER
	errorD ? true : TableTools::errorJudgerE(wordForErrorDE, nodeForErrorDE);
	// ERROR_E JUDGER

	return node;
}

// ＜值参数表＞ ::= ＜表达式＞{,＜表达式＞}｜＜空＞
SymbolNode* Parser::_值參數表() {
	SymbolNode* node = new SymbolNode(值參數表);
	if (word->getType() != RPARENT && word->getType() != SEMICN) {
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

	// ERROR_C JUDGER
	TableTools::errorJudgerC(word);
	// ERROR_C JUDGER END

	// ERROR_J JUDGER
	TableTools::errorJudgerJ(word);
	// ERROR_J JUDGER END

	IrGenerator::addScanIr(&word->getWord());
	node->addChild(_標識符());
	if (word->getType() == RPARENT) {
		node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
		getsym();
	}
	else {
		// ERROR_L JUDGER
		ErrorHandler::addErrorItem(ERROR_L, prevWord->getLine());
		// ERROR_L JUDGER END
	}
	return node;
}

// ＜写语句＞ ::= printf '(' ＜字符串＞,＜表达式＞ ')'| printf '('＜字符串＞ ')'| printf '('＜表达式＞')' 
SymbolNode* Parser::_寫語句() {
	int type = NOTYPE, num = 0;
	std::string* str = nullptr;
	SymbolNode* node = new SymbolNode(寫語句);
	node->addChild(new SymbolNode(word));	// word->getType() is PRINTFTK
	getsym();
	node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
	getsym();
	if (word->getType() == STRCON) {
		std::string* str = nullptr;
		node->addChild(_字符串(&str));
		if (word->getType() == COMMA) {
			IrGenerator::addPrintStrIr(str);
			node->addChild(new SymbolNode(word));	// word->getType() is COMMA
			getsym();
			node->addChild(_表達式(&type, &num, &str));
			IrGenerator::addPrintExpIr(type, num, str);
		}
		else {
			IrGenerator::addPrintStrIr(new std::string(*str + "\\n"));
		} // If no expression after string, add \n.
	}
	else {
		node->addChild(_表達式(&type, &num, &str));
		IrGenerator::addPrintExpIr(type, num, str);
	}
	if (word->getType() == RPARENT) {
		node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
		getsym();
	}
	else {
		// ERROR_L JUDGER
		ErrorHandler::addErrorItem(ERROR_L, prevWord->getLine());
		// ERROR_L JUDGER END
	}

	return node;
}

// ＜字符串＞ ::= "｛十进制编码为32,33,35-126的ASCII字符｝" 
SymbolNode* Parser::_字符串(std::string** str) {
	SymbolNode* node = new SymbolNode(字符串);

	// ERROR_A JUDGER
	if (word->getWord().size() == 0) {
		ErrorHandler::addErrorItem(ERROR_A, word->getLine());
	} // NO CHAR ERROR
	else {
		for (int i = 0; i < word->getWord().size(); i++) {
			char c = word->getWord().at(i);
			if (!((c >= 35 && c <= 126) || c == 32 || c == 33)) {
				ErrorHandler::addErrorItem(ERROR_A, word->getLine());
				break;
			} // ILLEGAL CHAR ERROR
		}
	}
	// ERROR_A JUDGER END

	*str = &word->getWord();
	node->addChild(new SymbolNode(word));
	getsym();
	return node;
}

// ＜返回语句＞ ::= return['('＜表达式＞')']   
SymbolNode* Parser::_返回語句() {
	SymbolNode* node = new SymbolNode(返回語句);
	if (word->getType() == RETURNTK) {
		node->addChild(new SymbolNode(word));
		getsym();
		if (word->getType() == LPARENT) {
			int retType; SymbolNode* nodeForErrorGH = nullptr;
			node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
			getsym();

			if (word->getType() == RPARENT) {
				// ERROR_GH JUDGER STAGE 2
				TableTools::errorJudgerGH(2, -1, word->getLine()); // "return ();" ERROR
				// ERROR_GH JUDGER STAGE 2 END
			}
			else {
				node->addChild((nodeForErrorGH = _表達式()));
			}

			if (word->getType() == RPARENT) {
				node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
				getsym();
			}
			else {
				// ERROR_L JUDGER
				ErrorHandler::addErrorItem(ERROR_L, prevWord->getLine());
				// ERROR_L JUDGER END
			}

			// ERROR_GH JUDGER STAGE 2
			if (nodeForErrorGH != nullptr) {
				retType = TableTools::isCharType(nodeForErrorGH) ? CHAR : INT;
				TableTools::errorJudgerGH(2, retType, word->getLine());
			}
			// ERROR_GH JUDGER STAGE 2 END

		}
		else {
			// ERROR_GH JUDGER STAGE 2
			TableTools::errorJudgerGH(2, VOID, word->getLine());
			// ERROR_GH JUDGER STAGE 2 END
		}
	}
	else error();
	return node;
}

// ＜情况语句＞ ::= switch ‘(’＜表达式＞‘)’ ‘{’＜情况表＞＜缺省＞‘}’
SymbolNode* Parser::_情況語句() {
	SymbolNode* nodeForErrorO;
	SymbolNode* node = new SymbolNode(情況語句);
	node->addChild(new SymbolNode(word));	// word->getType() is SWITCHTK
	getsym();
	node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
	getsym();
	node->addChild((nodeForErrorO = _表達式()));

	// ERROR_O JUDGER STAGE 1 
	TableTools::errorJudgerO(nodeForErrorO, 1);
	// ERROR_O JUDGER STAGE 1 END

	if (word->getType() == RPARENT) {
		node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
		getsym();
	}
	else {
		// ERROR_L JUDGER
		ErrorHandler::addErrorItem(ERROR_L, prevWord->getLine());
		// ERROR_L JUDGER END
	}
	node->addChild(new SymbolNode(word));	// word->getType() is LBRACE
	getsym();
	node->addChild(_情況表());
	if (word->getType() == DEFAULTTK) {
		node->addChild(_缺省());
	}
	else {
		// ERROR_P JUDGER
		ErrorHandler::addErrorItem(ERROR_P, word->getLine());
		// ERROR_P JUDGER END
	}
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
	SymbolNode* nodeForErrorO;
	SymbolNode* node = new SymbolNode(情況子語句);
	node->addChild(new SymbolNode(word));	// word->getType() is CASETK
	getsym();
	node->addChild((nodeForErrorO = _常量()));

	// ERROR_O JUDGER STAGE 2 
	TableTools::errorJudgerO(nodeForErrorO, 2);
	// ERROR_O JUDGER STAGE 2 END

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
	int type = NOTYPE, num = 0;
	std::string* str = nullptr, * res = nullptr;
	int line; SymbolNode* nodeForErrorI;
	SymbolNode* node = new SymbolNode(賦值語句);

	// ERROR_C JUDGER
	TableTools::errorJudgerC(word);
	// ERROR_C JUDGER END

	// ERROR_J JUDGER
	TableTools::errorJudgerJ(word);
	// ERROR_J JUDGER END

	res = &word->getWord();
	node->addChild(_標識符());
	if (word->getType() == LBRACK) {
		node->addChild(new SymbolNode(word));	// word->getType() is LBRACK
		getsym();
		// ERROR_I JUDGER
		line = word->getLine();
		node->addChild((nodeForErrorI = _表達式()));
		if (TableTools::isCharType(nodeForErrorI)) {
			ErrorHandler::addErrorItem(ERROR_I, line);
		}
		// ERROR_I JUDGER END
		if (word->getType() == RBRACK) {
			node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
			getsym();
		}
		else {
			// ERROR_M JUDGER
			ErrorHandler::addErrorItem(ERROR_M, prevWord->getLine());
			// ERROR_M JUDGER END
		}
		if (word->getType() == LBRACK) {
			node->addChild(new SymbolNode(word));	// word->getType() is LBRACK
			getsym();
			// ERROR_I JUDGER
			line = word->getLine();
			node->addChild((nodeForErrorI = _表達式()));
			if (TableTools::isCharType(nodeForErrorI)) {
				ErrorHandler::addErrorItem(ERROR_I, line);
			}
			// ERROR_I JUDGER END
			if (word->getType() == RBRACK) {
				node->addChild(new SymbolNode(word));	// word->getType() is RBRACK
				getsym();
			}
			else {
				// ERROR_M JUDGER
				ErrorHandler::addErrorItem(ERROR_M, prevWord->getLine());
				// ERROR_M JUDGER END
			}
		}
	}
	node->addChild(new SymbolNode(word));	// word->getType() is ASSIGN
	getsym();
	node->addChild(_表達式(&type, &num, &str));
	IrGenerator::addAssignIr(res, type, num, str);
	return node;
}

// ＜无返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')'
SymbolNode* Parser::_無返回值函數調用語句() {
	Word* wordForErrorDE = word;
	SymbolNode* nodeForErrorDE;
	SymbolNode* node = new SymbolNode(無返回值函數調用語句);
	node->addChild(_標識符());
	node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
	getsym();
	node->addChild((nodeForErrorDE = _值參數表()));		//node->addChild(_值參數表());
	if (word->getType() == RPARENT) {
		node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
		getsym();
	}
	else {
		// ERROR_L JUDGER
		ErrorHandler::addErrorItem(ERROR_L, prevWord->getLine());
		// ERROR_L JUDGER END
	}

	// ERROR_D JUDGER
	bool errorD = TableTools::errorJudgerD(wordForErrorDE, nodeForErrorDE);
	// ERROR_D JUDGER END

	// ERROR_E JUDGER
	errorD ? true : TableTools::errorJudgerE(wordForErrorDE, nodeForErrorDE);
	// ERROR_E JUDGER

	return node;
}

// ＜主函数＞ ::= void main‘(’‘)’ ‘{’＜复合语句＞‘}’
SymbolNode* Parser::_主函數() {

	// ERROR_GH JUDGER STAGE 1 
	TableTools::errorJudgerGH(1, VOID);
	// ERROR_GH JUDGER STAGE 1 END

	SymbolNode* node = new SymbolNode(主函數);
	node->addChild(new SymbolNode(word));	// word->getType() is VOIDTK
	getsym();
	node->addChild(new SymbolNode(word));	// word->getType() is MAINTK
	getsym();
	node->addChild(new SymbolNode(word));	// word->getType() is LPARENT
	getsym();
	if (word->getType() == RPARENT) {
		node->addChild(new SymbolNode(word));	// word->getType() is RPARENT
		getsym();
	}
	else {
		// ERROR_L JUDGER
		ErrorHandler::addErrorItem(ERROR_L, prevWord->getLine());
		// ERROR_L JUDGER END
	}
	node->addChild(new SymbolNode(word));	// word->getType() is LBRACE
	getsym();
	TableTools::addFunc(it - 1);
	node->addChild(_複合語句());
	node->addChild(new SymbolNode(word));	// word->getType() is RBRACE
	getsym();
	return node;
}