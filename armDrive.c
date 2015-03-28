/* Code copyright Harry Braviner March 2015

armDrive.c

Code to run on a PC and send commands over UART to
an ATmega controlling some servos

*/

#include <stdio.h>
#include <time.h>
#include <ncurses.h>

#define SETTING_ROW 4
#define SETTING_COL 10

int main(int argc, char *argv[]){

	uint16_t servoAsetting, servoBsetting;

	servoAsetting = 0;
	servoBsetting = 0;

	// Setup curses with just stdscr
	initscr();
	cbreak();
	keypad(stdscr, TRUE);
	noecho();
	curs_set(0);

	bool quit = false;
	int ch;

	while(!quit){
		// Draw the info
		mvprintw(SETTING_ROW,   SETTING_COL, "Servo A setting: %04x", servoAsetting);
		mvprintw(SETTING_ROW+1, SETTING_COL, "Servo B setting: %04x", servoBsetting);

		refresh();

		// Get a new character
		ch = getch();
		switch (ch){
			case 'q':
				quit = 1;
				break;
			case KEY_LEFT:
				if (servoAsetting != 0x0000)
					servoAsetting--;
				break;
			case KEY_RIGHT:
				if (servoAsetting != 0xFFFF)
					servoAsetting++;
				break;
			case KEY_DOWN:
				if (servoBsetting != 0x0000)
					servoBsetting--;
				break;
			case KEY_UP:
				if (servoBsetting != 0xffff)
					servoBsetting++;
				break;
		}
	}

	endwin();

	return 0;
}
