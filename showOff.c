// Designed to send a pretty pattern over UART
// Currently to light LEDs, eventually to move arm

#include <time.h>
#include <stdio.h>

//#define _POSIX_C_SOURCE 200809L

int main(int argc, char *argv[]){

	FILE *UART = fopen("/dev/ttyUSB0", "w");

	struct timespec delta;
	delta.tv_sec = 0;
	delta.tv_nsec = 10000000;
	
	// First light the LEDs gradually
	for (int i=0; i < 150; i++){
		fprintf(UART, "A%04x\n", (0xffff*i)/149);
		fprintf(UART, "B%04x\n", (0xffff*i)/149);
		nanosleep(&delta, NULL);
	}
	
	// Then dim the LEDs gradually
	for (int i=149; i > 0; i--){
		fprintf(UART, "A%04x\n", (0xffff*i)/149);
		fprintf(UART, "B%04x\n", (0xffff*i)/149);
		nanosleep(&delta, NULL);
	}

	// Now alternate them forever
	while(1){
		for (int i=0; i<150; i++){
			fprintf(UART, "A%04x\n", (0xffff*i)/149);
			fprintf(UART, "B%04x\n", (0xffff*(149-i))/149);
		}
		for (int i=149; i>0; i--){
			fprintf(UART, "A%04x\n", (0xffff*i)/149);
			fprintf(UART, "B%04x\n", (0xffff*(149-i))/149);
		}
	}

	fclose(UART);

	return 0;
}
