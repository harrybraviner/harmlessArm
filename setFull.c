#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <strings.h>
#include <unistd.h>

#define BAUDRATE B9600
#define TTYDEVICE "/dev/ttyUSB0"
#define _POSIX_SOURCE 1

int main(int argc, char *argv[]){

	int fd;	// Our tty device
	struct termios oldtio, newtio;	// Used to hold settings
	
	fd = open(TTYDEVICE, O_RDWR | O_NOCTTY);
	if(fd < 0){
		fprintf(stderr, "Failed to open %s. Exitting.\n", TTYDEVICE);
		return -1;
	}

	tcgetattr(fd, &oldtio);					// Save the current attributes
	bzero(&newtio, sizeof(newtio));	// Wipe the new settings

	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL;

	newtio.c_oflag = 0;				// Raw output
	newtio.c_lflag = ICANON;	// No echo, don't send signals to calling program

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	char *tBuf = malloc(7*sizeof(char));
	strcpy(tBuf, "\n");
	write(fd, tBuf, 1);
	strcpy(tBuf, "AFFFF\n");
	write(fd, tBuf, 6);
	strcpy(tBuf, "BFFFF\n");
	write(fd, tBuf, 6);

	tcsetattr(fd, TCSANOW, &oldtio);

	close(fd);

	return 0;
}
