#include "error.h"
#include <stdio.h>
#include <stdlib.h>

void error(const char *str) {
	puts(str);
	exit(-1);
}

void error() {
	puts("Error!");
	exit(-1);
}