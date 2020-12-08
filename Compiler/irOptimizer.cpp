#include "irOptimizer.h"
#include <cmath>
#include <cassert>
#include <sstream>
#include "table.h"

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
				itB != IrList.end() && (*itB)->getOp() != IR_FUNCDEF && (*itB)->getOp() != IR_LABEL &&
				(*itB)->getOp() != IR_BZ && (*itB)->getOp() != IR_BNZ && 
				(*itB)->getOp() != IR_PRECALL && (*itB)->getOp() != IR_GOTO && (*itB)->getOp() != IR_RET; ++itB) {
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

std::string* IrOptimizer::inlineLabelGen() {
	static int i = 0;
	std::string prefix = "Inline_";
	std::string suffix;
	std::stringstream ss;
	ss << i++;
	ss >> suffix;
	return new std::string(prefix + suffix);
}

void IrOptimizer::inLine() {
	std::vector<IrItem*> IrListCopy(IrList);
	IrList.clear();

	std::vector<std::vector<IrItem*>*> functionSet;
	std::vector<IrItem*>* function = nullptr;

	// Split IrList to several functions
	for (std::vector<IrItem*>::iterator it = IrListCopy.begin(); it != IrListCopy.end(); ++it) {
		IrItem* ir = *it;
		if (ir->getOp() == IR_FUNCDEF) {
			function = new std::vector<IrItem*>();
			functionSet.push_back(function);
		}
		assert(function != nullptr);
		function->push_back(ir);
	}

	// Set inline true for functions that has less than 4 local non-array vars (include params) and no function call
	for (std::vector<std::vector<IrItem*>*>::iterator it = functionSet.begin(); it != functionSet.end(); ++it) {
		function = *it;
		TableItem* ti = TableTools::search(function->at(0)->getLop(), 0);
		assert(ti->getType() == FUNC);
		bool hasArray = false;	int localsNum = 0;
		for (int i = 0; i < table.size(); i++) {
			if (table[i]->isSameScope(ti->getScopeInside())/* && table[i]->getType() != PARAM*/) {
				hasArray = (table[i]->getDimension() > 0) | hasArray;
				localsNum++;
			}
		}
		bool hasFunctionCall = false;
		for (int i = 0; i < function->size(); i++) {
			hasFunctionCall = (function->at(i)->getOp() == IR_CALL) | hasFunctionCall;
		}
		ti->setInline(!hasFunctionCall && !hasArray && localsNum <= 4 && !ti->isSameName(new std::string("main")));
		// function that has less than 4 local non-array vars (include params) and no function call, set inline true
	}

	// Regenerate IR
	for (std::vector<std::vector<IrItem*>*>::iterator it = functionSet.begin(); it != functionSet.end(); ++it) {
		function = *it;
		TableItem* ti = TableTools::search(function->at(0)->getLop(), 0);
		// Add Every not inlined function back to IrList
		if (!ti->isInline()) {
			for (int i = 0; i < function->size(); i++) {
				IrItem* ir = function->at(i);
				// If Not Precall IR or Precall IR but not inline function
				if (ir->getOp() != IR_PRECALL ||
					!TableTools::search(ir->getLop(), 0)->isInline()) {
					IrList.push_back(ir);
				}
				// Else, if need inlined
				else {
					std::string* inlineLabel = inlineLabelGen();
					IrList.push_back(new IrItem(IR_INLINE, IDTYPE, NOTYPE, 
						0, 0, function->at(i)->getLop(), nullptr, inlineLabel));
					std::vector<IrItem*>* inlineFunction = nullptr;
					// Get Inline Function's IR List
					for (int j = 0; j < functionSet.size(); j++) {
						if (*functionSet[j]->at(0)->getLop() == *ir->getLop()) {
							//inlineFunction = new std::vector<IrItem*>(*functionSet[j]);
							inlineFunction = functionSet[j];
						}
					}
					assert(inlineFunction);	// inlineFunction is not nullptr
					int iOfPrecall = i;	// in order to calculate the params' num
					// Add PUSH_INLINE IR to IrList
					for (i++; (ir = function->at(i))->getOp() != IR_CALL; i++) {
						assert(ir->getOp() == IR_PUSH);
						IrList.push_back(new IrItem(IR_PUSH_IL, ir->getLopType(), INTTYPE,
							ir->getLopInt(), i - iOfPrecall - 1, ir->getLop(), nullptr, nullptr));
					}
					// Add All inline function's IR to IrList
					for (int j = 0; j < inlineFunction->size(); j++) {
						if (inlineFunction->at(j)->getOp() == IR_FUNCDEF) {
							IrItem* newIr = new IrItem(*inlineFunction->at(j));
							newIr->setOp(IR_FUNCDEF_IL);
							IrList.push_back(newIr);
						}
						else if (inlineFunction->at(j)->getOp() == IR_RET) {
							IrItem* newIr = new IrItem(*inlineFunction->at(j));
							newIr->setOp(IR_RETURN_IL);
							newIr->setRes(inlineLabel);
							IrList.push_back(newIr);
						}
						else if (inlineFunction->at(j)->getOp() == IR_LABEL || 
							     inlineFunction->at(j)->getOp() == IR_GOTO || 
							     inlineFunction->at(j)->getOp() == IR_BNZ || 
							     inlineFunction->at(j)->getOp() == IR_BZ ) {
							IrItem* newIr = new IrItem(*inlineFunction->at(j));
							newIr->setRes(new std::string(*newIr->getRes() + "_in_" + *inlineLabel));
							IrList.push_back(newIr);
						}
						else {
							IrList.push_back(inlineFunction->at(j));
						}
					}
					// IrList.insert(IrList.end(), inlineFunction->begin() + 1, inlineFunction->end());
					// Add OUTLINE IR to IrList
					IrList.push_back(new IrItem(IR_OUTLINE, IDTYPE, NOTYPE,
						0, 0, function->at(i)->getLop(), ir->getRes(), inlineLabel));
					// lop - function name, rop - ret value label, res - inline label
				}
			}
		}
	}
	return;
}

void IrOptimizer::optimize() {
	tempPropagation();
	mul2sll();
	div2sra();
	//inLine();
}