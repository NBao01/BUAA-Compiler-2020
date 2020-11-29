#include "irOptimizer.h"
#include <cmath>

void IrOptimizer::mul2sll() {
	std::vector<IrItem*> IrListCopy(IrList);
	IrList.clear();
	std::vector<IrItem*>::iterator it;
	for (it = IrListCopy.begin(); it != IrListCopy.end(); ++it) {
		IrItem* ir = *it;
		if (ir->getOp() == IR_MUL && ir->getLopType() == INTTYPE && ir->getLopInt() != 0 &&
			(ir->getLopInt() & (ir->getLopInt() - 1)) == 0) {
			IrList.push_back(new IrItem(IR_SLL, ir->getRopType(), INTTYPE,
				ir->getRopInt(), (int)log2(ir->getLopInt()), ir->getRop(), nullptr, ir->getRes()));
		}
		else if (ir->getOp() == IR_MUL && ir->getRopType() == INTTYPE && ir->getRopInt() != 0 &&
			(ir->getRopInt() & (ir->getRopInt() - 1)) == 0) {
			IrList.push_back(new IrItem(IR_SLL, ir->getLopType(), INTTYPE,
				ir->getLopInt(), (int)log2(ir->getRopInt()), ir->getLop(), nullptr, ir->getRes()));
		}
		else {
			IrList.push_back(ir);
		}
	}
}

void IrOptimizer::optimize() {
	mul2sll();
}