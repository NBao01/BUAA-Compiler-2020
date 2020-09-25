#include "lexicalAnalyzer.h"
#include "stateMachine.h"
#include "keywordJudger.h"
#include "tokens.h"
#include "error.h"
#include <fstream>
#include <cctype>
#include <cassert>

std::vector<Wordlist*> LexicalAnalyzer::wordlist;
char buf[100], c; // buf is the buffer of each word, c is the character read from the file.
int type = 0; // the type of each word.
int line = 1; // the line of the input file.
int pos = 0; // the recording position in buf.

void LexicalAnalyzer::analyze() {
	//FILE* in = fopen("testfile.txt", "r");
	std::ifstream in("testfile.txt", std::ios_base::in);
	while (in.peek() != EOF) {
		if (!(
			type == IDENFR || type == INTCON || 
			type == LSS || type == GRE || type == ASSIGN || 
			(type >= CONSTTK && type <= RETURNTK)
			)) {
			//c = fgetc(in);
			in.get(c);
		} // These are the tokens that need to ungetc (a.k.a. hold the char c for next word).

		type = StateMachine::transit(c); // make transition

		if (type == -GENERAL_ERROR) {
			printf("ERROR IN StateMachine::transit!\n");
			return;
		}
		else if (type == 0) {
			buf[pos++] = c;
		} // The State Machine is Reading The Word.
		else if (type != 0) {
			switch (type) {
			case IDENFR:
				buf[pos] = '\0';
				type = KeywordJudger::isKeyword(buf);
				wordlist.push_back(new Wordlist(buf, type, line));
				break;
			case INTCON:
				buf[pos] = '\0';
				wordlist.push_back(new Wordlist(buf, type, line));
				break;
			case CHARCON:
				assert(pos == 2); // buf[0] is '\'', buf[1] is THE char.
				buf[pos] = '\0';
				wordlist.push_back(new Wordlist(&buf[1], type, line));
				break;
			case STRCON:
				buf[pos] = '\0';
				wordlist.push_back(new Wordlist(&buf[1], type, line));
				break;
			case PLUS:
				wordlist.push_back(new Wordlist("+", type, line));
				break;
			case MINU:
				wordlist.push_back(new Wordlist("-", type, line));
				break;
			case MULT:
				wordlist.push_back(new Wordlist("*", type, line));
				break;
			case DIV:
				wordlist.push_back(new Wordlist("/", type, line));
				break;
			case LSS:
				wordlist.push_back(new Wordlist("<", type, line));
				break;
			case LEQ:
				wordlist.push_back(new Wordlist("<=", type, line));
				break;
			case GRE:
				wordlist.push_back(new Wordlist(">", type, line));
				break;
			case GEQ:
				wordlist.push_back(new Wordlist(">=", type, line));
				break;
			case EQL:
				wordlist.push_back(new Wordlist("==", type, line));
				break;
			case NEQ:
				wordlist.push_back(new Wordlist("!=", type, line));
				break;
			case COLON:
				wordlist.push_back(new Wordlist(":", type, line));
				break;
			case ASSIGN:
				wordlist.push_back(new Wordlist("=", type, line));
				break;
			case SEMICN:
				wordlist.push_back(new Wordlist(";", type, line));
				break;
			case COMMA:
				wordlist.push_back(new Wordlist(",", type, line));
				break;
			case LPARENT:
				wordlist.push_back(new Wordlist("(", type, line));
				break;
			case RPARENT:
				wordlist.push_back(new Wordlist(")", type, line));
				break;
			case LBRACK:
				wordlist.push_back(new Wordlist("[", type, line));
				break;
			case RBRACK:
				wordlist.push_back(new Wordlist("]", type, line));
				break;
			case LBRACE:
				wordlist.push_back(new Wordlist("{", type, line));
				break;
			case RBRACE:
				wordlist.push_back(new Wordlist("}", type, line));
				break;
			}
			pos = 0;
		}


		if (c == '\n') {
			line++;
		}
	}
	in.close();
}

void LexicalAnalyzer::output() {
	//FILE *out = fopen("output.txt", "w");
	std::ofstream out("output.txt", std::ios_base::out);
	std::vector<Wordlist*>::iterator it;
	for (it = wordlist.begin(); it != wordlist.end(); ++it) {
		//fprintf(out, "%s %s\n", tokens[(* it)->getType()], (* it)->getWord());
		out << tokens[(*it)->getType()] << " " << (*it)->getWord() << std::endl;
	}
	out.close();
}