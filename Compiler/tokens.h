#ifndef __TOKENS_H__
#define __TOKENS_H__

#include <string>

#define UNKNOWN		0
#define IDENFR		1
#define INTCON		2
#define CHARCON		3
#define STRCON		4
#define CONSTTK		5
#define INTTK		6
#define CHARTK		7
#define VOIDTK		8
#define MAINTK		9
#define IFTK		10
#define ELSETK		11
#define SWITCHTK	12
#define CASETK		13
#define DEFAULTTK	14
#define WHILETK		15
#define FORTK		16
#define SCANFTK		17
#define PRINTFTK	18
#define RETURNTK	19
#define PLUS		20
#define MINU		21
#define MULT		22
#define DIV			23
#define LSS			24
#define LEQ			25
#define GRE			26
#define GEQ			27
#define EQL			28
#define NEQ			29
#define COLON		30
#define ASSIGN		31
#define SEMICN		32
#define COMMA		33
#define LPARENT		34
#define RPARENT		35
#define LBRACK		36
#define RBRACK		37
#define LBRACE		38
#define RBRACE		39

#define WHITESPACE	40	// FOR WHITESPACE FILTER

extern const std::string tokens[40];

#endif // !__TOKENS_H__