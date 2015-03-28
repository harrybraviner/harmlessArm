CC=/usr/bin/avr-gcc
CFLAGS=-g -Os -Wall -mmcu=atmega$(MEGA) -std=c99
OBJ2HEX=/usr/bin/avr-objcopy
MEGA=328p
TARGET=harmlessArm
PROG=/usr/bin/avrdude
PROGDEVICE=/dev/ttyACM0

program : $(TARGET).hex
	$(PROG) -c avrisp2 -p m$(MEGA) -P $(PROGDEVICE) -e
	$(PROG) -c avrisp2 -p m$(MEGA) -P $(PROGDEVICE) -U flash:w:$(TARGET).hex

%.obj : %.o
	$(CC) $(CFLAGS) $< -o $@

%.hex : %.obj
	$(OBJ2HEX) -R .eeprom -O ihex $< $@

setFull : setFull.c
	gcc -Wall setFull.c -o setFull

showOff : showOff.c
	gcc -Wall showOff.c -o showOff -std=gnu99

armDrive : armDrive.c
	gcc -Wall armDrive.c -o armDrive -std=gnu99 -lncurses
