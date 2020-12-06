#ifndef __INSTRUCTIONDEFINITIONS_H__
#define __INSTRUCTIONDEFINITIONS_H__

#include <string>

#define IR_ADD		1
#define IR_SUB		2
#define IR_MUL		3
#define IR_DIV		4
#define IR_PRINT	5
#define IR_SCAN		6
#define IR_ASSIGN	7
#define IR_FUNCDEF	8
#define IR_PUSH		9
#define IR_CALL		10
#define IR_RET		11
#define IR_GOTO		12
#define IR_BNZ		13
#define IR_BZ		14
#define IR_LSS		15
#define IR_LEQ		16
#define IR_GRE		17
#define IR_GEQ		18
#define IR_EQL		19
#define IR_NEQ		20
#define IR_LABEL	21
#define IR_ARRAYGET 22
#define IR_ARRAYSET 23
#define IR_PRECALL	24
#define IR_SLL		25
#define IR_SRA		26
#define IR_INLINE		27
#define IR_OUTLINE		28	// Inline end
#define IR_PUSH_IL		29	// Push for inline
#define IR_FUNCDEF_IL	30	// Function define for inline
#define IR_RETURN_IL	31	// Return for inline

extern const std::string irInstructions[];

#endif // !__INSTRUCTIONDEFINITIONS_H__
