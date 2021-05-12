#include "StdAfx.h"
#include <stdio.h>

void PD (int i) {
	FILE *debugfile;
	debugfile = fopen("VsamDeb", "a");
	fprintf(debugfile, "%d ", i);
	fclose(debugfile);
}
