#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>

const std::string errors[18] = {
	"", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "OTHER_ERROR"
};

std::vector<ErrorItem*> errorList;

ErrorItem::ErrorItem(int type, int line) {
	this->type = type > 0 ? type : -type;
	this->line = line;
}

int ErrorItem::getType() {
	return type;
}

int ErrorItem::getLine() {
	return line;
}

void ErrorHandler::addErrorItem(ErrorItem* e) {
	errorList.push_back(e);
}

void ErrorHandler::addErrorItem(int type, int line) {
	addErrorItem(new ErrorItem(type, line));
}

void ErrorHandler::output() {
	std::ofstream out("error.txt", std::ios_base::out);
	std::vector<ErrorItem*>::iterator it;
	for (it = errorList.begin(); it != errorList.end(); ++it) {
		out << (*it)->getLine() << " " << errors[(*it)->getType()] << std::endl;
	}
	out.close();
}

void error(const char *str) {
	puts(str);
	exit(-1);
}

void error() {
	puts("Error!");
	exit(-1);
}