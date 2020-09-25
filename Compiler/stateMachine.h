#ifndef __STATEMACHINE_H__
#define __STATEMACHINE_H__

#define INITIAL_STATE		0
#define IDENFR_STATE		1
#define INTCON_STATE		2
#define STRCON_STATE		3
#define CHARCON_STATE_1		4
#define CHARCON_STATE_2		5
#define LSS_STATE			6
#define GRE_STATE			7
#define ASSIGN_STATE		8
#define BANG_STATE			9
#define TRAP_STATE			10

class StateMachine {
private:
	static int state;
public:
	static int transit(char c);
};

#endif // !__STATEMACHINE_H__