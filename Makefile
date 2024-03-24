all:
	@echo nothing

main: main/main.touch

main/main.touch: main/main.ino 
	arduino-cli compile -b arduino:avr:uno main
	touch main/main.touch

inst: main/main.touch 
	arduino-cli upload -p /dev/ttyUSB0 -b arduino:avr:uno main

clean: 
	rm main/main.touch 

lowcore: lowcore/lowcore.touch

lowcore/lowcore.touch: lowcore/lowcore.ino
	arduino-cli compile -b arduino:avr:uno lowcore
	touch lowcore/lowcore.touch

ardeeprom: ardeeprom/ardeeprom.touch

ardeeprom/ardeeprom.touch: ardeeprom/ardeeprom.ino
	arduino-cli compile -b arduino:avr:uno ardeeprom
	touch ardeeprom/ardeeprom.touch
