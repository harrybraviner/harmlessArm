// Designed to send a pretty pattern over UART
// Currently to light LEDs, eventually to move arm

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>

#define TTYDEVICE "/dev/ttyUSB0"

int main(int argc, char *argv[]){

	int uart = open(TTYDEVICE, O_WRONLY | O_NOCTTY);
	if (uart < 0){
		fprintf(stderr, "Unable to open %s for writing. Exitting.\n", TTYDEVICE);
		return -1;
	}

	struct termios oldtio, newtio;
	tcgetattr(uart, &oldtio);				// Save the old settings

	bzero(&newtio, sizeof(newtio));	// Wipe the new settings
	newtio.c_cflag = B9600 | CS8 | CLOCAL;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	tcflush(uart, TCIFLUSH);
	tcsetattr(uart, TCSANOW, &newtio);

	char *buffTX;
	buffTX = malloc(7*sizeof(char));
	strcpy(buffTX, "00000\n");

	struct timespec delta;
	delta.tv_sec = 0;
	delta.tv_nsec = 10000000;
	
	// First send a newline - we seem to need this for some reason!
	sprintf(buffTX, "\n");
	write(uart, buffTX, 1);

	// First light the LEDs gradually
	for (int i=0; i < 150; i++){
		sprintf(buffTX, "A%04x\n", (0xffff*i)/149); 
		write(uart, buffTX, 6);
		sprintf(buffTX, "B%04x\n", (0xffff*i)/149);
		write(uart, buffTX, 6);
		nanosleep(&delta, NULL);
	}

	// Then dim the LEDs gradually
	for (int i=149; i > 0; i--){
		sprintf(buffTX, "A%04x\n", (0xffff*i)/149);
		write(uart, buffTX, 6);
		sprintf(buffTX, "B%04x\n", (0xffff*i)/149);
		write(uart, buffTX, 6);
		nanosleep(&delta, NULL);
	}

	// Now alternate them forever
	while(1){
		for (int i=0; i<150; i++){
			sprintf(buffTX, "A%04x\n", (0xffff*i)/149);
			write(uart, buffTX, 6);
			sprintf(buffTX, "B%04x\n", (0xffff*(149-i))/149);
			write(uart, buffTX, 6);
		}
		for (int i=149; i>0; i--){
			sprintf(buffTX, "A%04x\n", (0xffff*i)/149);
			write(uart, buffTX, 6);
			sprintf(buffTX, "B%04x\n", (0xffff*(149-i))/149);
			write(uart, buffTX, 6);
		}
	}

	tcsetattr(uart, TCSANOW, &oldtio);	// Restore the original settings
	close(uart);

	return 0;
}
