/* Code copyright Harry Braviner March 2015

armDrive.c

Code to run on a PC and send commands over UART to
an ATmega controlling some servos

*/

#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <ncurses.h>

#define TTYDEVICE "/dev/ttyUSB0"

#define SETTING_ROW 4
#define SETTING_COL 10

#define HELP_ROW 8
#define HELP_COL 5

#define ACCEL_A 70
#define ACCEL_B 70
#define DECCEL_A 210
#define DECCEL_B 210

int16_t servoArate, servoBrate;
uint16_t servoAposition, servoBposition;

void updateServos();

int main(int argc, char *argv[]){

	/* Begin setting up UART */
	int uart = open(TTYDEVICE, O_WRONLY | O_NOCTTY);
	if (uart < 0){
		fprintf(stderr, "Unable to open %s for writing. Exitting\n", TTYDEVICE);
		return -1;
	}

	struct termios oldtio, newtio;
	char *txBuffer = malloc(7*sizeof(char));

	// Save the old attributes
	tcgetattr(uart, &oldtio);

	// Set the new attributes
	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = B9600 | CS8 | CLOCAL;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	tcflush(uart, TCIFLUSH);
	tcsetattr(uart, TCSANOW, &newtio);
	strcpy(txBuffer, "\n");
	write(uart, txBuffer, 1);
	/* Finished setting up UART */

	servoAposition = 0;
	servoBposition = 0;

	// Setup curses with just stdscr
	initscr();
	cbreak();
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);
	noecho();
	curs_set(0);
	// Draw help text
	mvprintw(HELP_ROW,   HELP_COL, "To move servo A: right and left arrow keys");
	mvprintw(HELP_ROW+1, HELP_COL, "To move servo B: up    and down arrow keys");
	mvprintw(HELP_ROW+2, HELP_COL, "To quit, press q");

	bool quit = false;
	int ch;

	struct timespec delta;	// Waiting time between inputs
	delta.tv_sec = 0;
	delta.tv_nsec = 50000000;

	while(!quit){
		// Draw the info
		mvprintw(SETTING_ROW,   SETTING_COL, "Servo A position: %04x", servoAposition);
		mvprintw(SETTING_ROW+1, SETTING_COL, "Servo B position: %04x", servoBposition);

		refresh();

		// Get a new character
		nanosleep(&delta, NULL);
		ch = getch();
		if (ch == 'q'){
			quit = 1;
			break;
		}

#if 0
		if (ch == ERR){
			mvprintw(SETTING_ROW+3, SETTING_COL+10, "Error!\n");
		} else {
			mvprintw(SETTING_ROW+3, SETTING_COL+10, "No error :D\n");
		}
#endif

#if 1
		updateServos(ch);

		// Write the new servo values over UART
		sprintf(txBuffer, "A%04x\n", servoAposition);
		write(uart, txBuffer, 6);
		sprintf(txBuffer, "B%04x\n", servoBposition);
		write(uart, txBuffer, 6);
#endif
	}

	// Restore original setting and close UART filestream
	tcsetattr(uart, TCSANOW, &oldtio);
	close(uart);
	
	endwin();

	return 0;
}

void updateServos(int ch){
	// Update the movement rates
	if(ch == KEY_LEFT){
		if (servoArate > -10000)
			servoArate -= ACCEL_A;
	}
	else if(ch == KEY_RIGHT){
		if (servoArate < +10000)
			servoArate += ACCEL_A;
	}
	else {
		if (servoArate > 0){
			if (servoArate >= +DECCEL_A) servoArate -= DECCEL_A;
			else                        servoArate = 0;
		} else if (servoArate < 0){
			if (servoArate <= -DECCEL_A) servoArate += DECCEL_A;
			else                        servoArate = 0;
		}
	}
	if(ch == KEY_DOWN){
		if (servoBrate > -10000)
			servoBrate -= ACCEL_B;
	}
	else if(ch == KEY_UP){
		if (servoBrate < +10000)
			servoBrate += ACCEL_B;
	}
	else {
		if (servoBrate > 0){
			if (servoBrate >= +DECCEL_B) servoBrate -= DECCEL_B;
			else                        servoBrate = 0;
		} else if (servoBrate < 0){
			if (servoBrate <= -DECCEL_B) servoBrate += DECCEL_B;
			else                        servoBrate = 0;
		}
	}

	// Ensure we aren't going to run overflow an end-stop
	if (servoArate < 0 && servoAposition < -servoArate){
		servoArate = -servoAposition;
	} else if (servoArate > 0 && (0xffff - servoAposition) < servoArate) {
		servoArate = 0xffff - servoAposition;
	}
	if (servoBrate < 0 && servoBposition < -servoBrate){
		servoBrate = -servoBposition;
	} else if (servoBrate > 0 && (0xffff - servoBposition) < servoBrate) {
		servoBrate = 0xffff - servoBposition;
	}

	// Update the positions
	servoAposition += servoArate;
	servoBposition += servoBrate;
}
