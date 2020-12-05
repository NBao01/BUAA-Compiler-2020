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

void IrOptimizer::div2sra() {
	std::vector<IrItem*> IrListCopy(IrList);
	IrList.clear();
	std::vector<IrItem*>::iterator it;
	for (it = IrListCopy.begin(); it != IrListCopy.end(); ++it) {
		IrItem* ir = *it;
		if (ir->getOp() == IR_DIV && ir->getRopType() == INTTYPE && ir->getRopInt() == 2) {
			std::string* newLabel1 = IrGenerator::tempIdentifierGen();
			std::string* newLabel2 = IrGenerator::tempIdentifierGen();
			IrList.push_back(new IrItem(IR_SRA, ir->getLopType(), INTTYPE, 
				ir->getLopInt(), 31, ir->getLop(), nullptr, newLabel1));
			IrList.push_back(new IrItem(IR_SUB, ir->getLopType(), TMPTYPE,
				ir->getLopInt(), 0, ir->getLop(), newLabel1, newLabel2));
			IrList.push_back(new IrItem(IR_SRA, TMPTYPE, INTTYPE, 0, 1, newLabel2, nullptr, ir->getRes()));
		}
		else {
			IrList.push_back(ir);
		}
	}
}

void IrOptimizer::tempPropagation() {
	std::vector<IrItem*>::iterator it;
	for (it = IrList.begin(); it != IrList.end(); ) {
		IrItem* ir = *it;
		if ((ir->getOp() == IR_ADD || ir->getOp() == IR_SUB || ir->getOp() == IR_MUL || ir->getOp() == IR_DIV) &&
			ir->getLopType() == INTTYPE && ir->getRopType() == INTTYPE && ir->getRes()->at(0) == '$') {
			std::string* label = ir->getRes();
			int num = 0;
			switch (ir->getOp()) {
			case IR_ADD:
				num = ir->getLopInt() + ir->getRopInt();
				break;
			case IR_SUB:
				num = ir->getLopInt() - ir->getRopInt();
				break;
			case IR_MUL:
				num = ir->getLopInt() * ir->getRopInt();
				break;
			case IR_DIV:
				num = ir->getLopInt() / ir->getRopInt();
				break;
			}
			for (std::vector<IrItem*>::iterator itB = it + 1;
				itB != IrList.end() && (*itB)->getOp() != IR_LABEL &&
				(*itB)->getOp() != IR_BZ && (*itB)->getOp() != IR_BNZ; ++itB) {
				IrItem* irB = *itB;
				if (irB->getLopType() == TMPTYPE && *irB->getLop() == *label) {
					irB->setLopType(INTTYPE);
					irB->setLopInt(num);
					irB->setLop(nullptr);
				}
				if (irB->getRopType() == TMPTYPE && *irB->getRop() == *label) {
					irB->setRopType(INTTYPE);
					irB->setRopInt(num);
					irB->setRop(nullptr);
				}
			}
			it = IrList.erase(it);
		}
		else {
			++it;
		}
	}
}

void IrOptimizer::optimize() {
	tempPropagation();
	mul2sll();
	div2sra();
}