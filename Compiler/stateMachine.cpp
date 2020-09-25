#include "tokens.h"
#include "stateMachine.h"
#include "error.h"
#include <cctype>

int StateMachine::state = INITIAL_STATE;

/* The State Machine's States Transition.
* pass a char as trigger to transit.
* return the macro in tokens.h if the machine reach the final states.
* return 0 if the machine is in the intermediate states.
* return -GENERAL_ERROR if the machine is trapped in error state.
*/
int StateMachine::transit(char c) {
	if (state == INITIAL_STATE) {
		if (isalpha(c)) {
			state = IDENFR_STATE;
		}
		else if (isdigit(c)) {
			state = INTCON_STATE;
		}
		else if (c == '\"') {
			state = STRCON_STATE;
		}
		else if (c == '\'') {
			state = CHARCON_STATE_1;
		}
		else if (c == '+') { return PLUS; }
		else if (c == '-') { return MINU; }
		else if (c == '*') { return MULT; }
		else if (c == '/') { return DIV; }
		else if (c == '<') {
			state = LSS_STATE;
		}
		else if (c == '>') {
			state = GRE_STATE;
		}
		else if (c == '=') {
			state = ASSIGN_STATE;
		}
		else if (c == '!') {
			state = BANG_STATE;
		}
		else if (c == ':') { return COLON; }
		else if (c == ';') { return SEMICN; }
		else if (c == ',') { return COMMA; }
		else if (c == '(') { return LPARENT; }
		else if (c == ')') { return RPARENT; }
		else if (c == '[') { return LBRACK; }
		else if (c == ']') { return RBRACK; }
		else if (c == '{') { return LBRACE; }
		else if (c == '}') { return RBRACE; }
		else if (isspace(c)) { return WHITESPACE; } // FOR WHITESPACES FILTER
	}
	else if (state == IDENFR_STATE) {
		if (isalpha(c) || isdigit(c) || c == '_') {
			state = IDENFR_STATE;
		}
		else {
			state = INITIAL_STATE;
			return IDENFR;
		}
	}
	else if (state == INTCON_STATE) {
		if (isdigit(c)) {
			state = INTCON_STATE;
		}
		else {
			state = INITIAL_STATE;
			return INTCON;
		}
	}
	else if (state == STRCON_STATE) {
		if ((c >= 35 && c <= 126) || c == 32 || c == 33) {
			state = STRCON_STATE;
		}
		else if (c == '\"') {
			state = INITIAL_STATE;
			return STRCON;
		}
		else {
			state = TRAP_STATE;
			return -GENERAL_ERROR;
		}
	}
	else if (state == CHARCON_STATE_1) {
		if (isalpha(c) || isdigit(c) || 
			c == '+' || c == '-' || c == '*' || c == '/' || c == '_') {
			state = CHARCON_STATE_2;
		}
		else {
			state = TRAP_STATE;
			return -GENERAL_ERROR;
		}
	}
	else if (state == CHARCON_STATE_2) {
		if (c == '\'') {
			state = INITIAL_STATE;
			return CHARCON;
		}
		else {
			state = TRAP_STATE;
			return -GENERAL_ERROR;
		}
	}
	else if (state == LSS_STATE) {
		if (c == '=') {
			state = INITIAL_STATE;
			return LEQ;
		}
		else {
			state = INITIAL_STATE;
			return LSS;
		}
	}
	else if (state == GRE_STATE) {
		if (c == '=') {
			state = INITIAL_STATE;
			return GEQ;
		}
		else {
			state = INITIAL_STATE;
			return GRE;
		}
	}
	else if (state == ASSIGN_STATE) {
		if (c == '=') {
			state = INITIAL_STATE;
			return EQL;
		}
		else {
			state = INITIAL_STATE;
			return ASSIGN;
		}
	}
	else if (state == BANG_STATE) {
		if (c == '=') {
			state = INITIAL_STATE;
			return NEQ;
		}
		else {
			state = TRAP_STATE;
			return -GENERAL_ERROR;
		}
	}
	return 0;
}

/* Tokens need to ungetc:
* IDENFR, INTCON, LSS, GRE, ASSIGN
*/