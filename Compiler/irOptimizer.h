#ifndef __IROPTIMIZER_H__
#define __IROPTIMIZER_H__

#include "ir.h"
#include "irDefinitions.h"

class IrOptimizer {
private:
	static void mul2sll();
public:
	static void optimize();
};

#endif // !__IROPTIMIZER_H__
