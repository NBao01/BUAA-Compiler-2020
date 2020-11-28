#ifndef __MIPSDEFINITIONS_H__
#define __MIPSDEFINITIONS_H__

#include <string>

// MIPS INSTRUCTIONS
#define MIPS_ADD		1
#define MIPS_SUB		2
#define MIPS_MUL		3
#define MIPS_DIV		4
#define MIPS_LW			5
#define MIPS_SW			6
#define MIPS_LI			7
#define MIPS_LA			8
#define MIPS_SYSCALL	9
#define MIPS_LABEL		10
#define MIPS_ADDI		11
#define MIPS_J			12
#define MIPS_JAL		13
#define MIPS_JR			14
#define MIPS_BLT		15
#define MIPS_BLE		16
#define MIPS_BGT		17
#define MIPS_BGE		18
#define MIPS_BEQ		19
#define MIPS_BNE		20
#define MIPS_SLL		21

// REGISTER FILE
#define $zero	0
#define $at		1
#define $v0		2
#define $v1		3
#define $a0		4
#define $a1		5
#define $a2		6
#define $a3		7
#define $t0		8
#define $t1		9
#define $t2		10
#define $t3		11
#define $t4		12
#define $t5		13
#define $t6		14
#define $t7		15
#define $s0		16
#define $s1		17
#define $s2		18
#define $s3		19
#define $s4		20
#define $s5		21
#define $s6		22
#define $s7		23
#define $t8		24
#define $t9		25
#define $k0		26
#define $k1		27
#define $gp		28
#define $sp		29
#define $fp		30
#define $ra		31

// DATA TYPE IN ASM
#define _WORD	1
#define _ASCIIZ	2
#define __SPACE	3

extern const std::string mipsInstructions[];
extern const std::string regstr[];

#endif // !__MIPSDEFINITIONS_H__

