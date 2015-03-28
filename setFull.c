#include <stdio.h>

int main(int argc, char *argv[]){
	FILE *UART = fopen("/dev/ttyUSB0", "w");

	fprintf(UART, "AFFFF\n");
	fprintf(UART, "BFFFF\n");

	fclose(UART);

	return 0;
}
