#ifndef __IROPTIMIZER_H__
#define __IROPTIMIZER_H__

#include "ir.h"
#include "irDefinitions.h"

class IrOptimizer {
private:
	static void mul2sll();
	static void div2sra();
	static void tempPropagation();
	static void inLine();
	static std::string* inlineLabelGen();
public:
	static void optimize();
};

#endif // !__IROPTIMIZER_H__
