﻿#ifndef __PARSER_H__
#define __PARSER_H__

#include "wordlist.h"
#include "abstractSyntaxTree.h"
#include "ir.h"
#include <vector>

void getsym();

class Parser {
public:
	static SymbolNode* _程序();
	static SymbolNode* _常量說明();
	static SymbolNode* _常量定義();
	static SymbolNode* _整數();
	static SymbolNode* _整數(int* num);
	static SymbolNode* _無符號整數();
	static SymbolNode* _無符號整數(int* num);
	static SymbolNode* _字符();
	static SymbolNode* _字符(std::string** str);
	static SymbolNode* _字符(int* ascii);
	static SymbolNode* _變量說明();
	static SymbolNode* _變量定義();
	static SymbolNode* _變量定義無初始化();
	static SymbolNode* _變量定義及初始化();
	static SymbolNode* _標識符(int* type, std::string** str);
	static SymbolNode* _標識符();
	static SymbolNode* _類型標識符();
	static SymbolNode* _常量();
	static SymbolNode* _常量(int* num);
	static SymbolNode* _有返回值函數定義();
	static SymbolNode* _無返回值函數定義();
	static SymbolNode* _聲明頭部(int* type, std::string** str);
	static SymbolNode* _聲明頭部();
	static SymbolNode* _參數表();
	static SymbolNode* _複合語句();
	static SymbolNode* _語句列();
	static SymbolNode* _語句();
	static SymbolNode* _循環語句();
	static SymbolNode* _條件語句();
	static SymbolNode* _條件();
	static SymbolNode* _表達式();
	static SymbolNode* _表達式(int* type, int* num, std::string** str);
	static SymbolNode* _步長(int* num);
	static SymbolNode* _關係運算符();
	static SymbolNode* _項();
	static SymbolNode* _項(int* type, int* num, std::string** str);
	static SymbolNode* _加法運算符();
	static SymbolNode* _因子();
	static SymbolNode* _因子(int* type, int* num, std::string** str);
	static SymbolNode* _乘法運算符();
	static SymbolNode* _有返回值函數調用語句();
	static SymbolNode* _有返回值函數調用語句(std::string** retLabel);
	static SymbolNode* _值參數表(std::vector<IrItem*>* pushList);
	static SymbolNode* _讀語句();
	static SymbolNode* _寫語句();
	static SymbolNode* _字符串(std::string** str);
	static SymbolNode* _返回語句();
	static SymbolNode* _情況語句();
	static SymbolNode* _情況表(int type, int num, std::string* str, std::vector<IrItem*>* before);
	static SymbolNode* _缺省(std::vector<IrItem*>* before);
	static SymbolNode* _情況子語句(int type, int num, std::string* str, std::vector<IrItem*>* before);
	static SymbolNode* _賦值語句();
	static SymbolNode* _無返回值函數調用語句();
	static SymbolNode* _主函數();
};

#endif // !__PARSER_H__

