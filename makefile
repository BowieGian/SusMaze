SOURCES=main.c game.c mya2d.c joystick.c timer.c joystick_input.c gpio.c audioMixer.c game_audio.c button.c menu.c game_display.c lcd_screen.c
TARGET=game
OUTDIR=$(HOME)/cmpt433/public/myApps/
PRJDIR=$(HOME)/cmpt433/work/FinalProject_Game/

CROSS_COMPILER=arm-linux-gnueabihf-
CC_C = $(CROSS_COMPILER)gcc
CFLAGS=-Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

LFLAGS = -L$(HOME)/cmpt433/public/asound_lib_BBB

all:
	$(CC_C) $(CFLAGS) $(SOURCES) -o $(OUTDIR)/$(TARGET)  $(LFLAGS) -lpthread -lasound
#	cp $(TARGET) $(OUTDIR)
	cp -r ./source-file $(OUTDIR)

clean:
#	rm $(PRJDIR)/$(TARGET)
	rm $(OUTDIR)/$(TARGET)
	rm -r $(OUTDIR)/source-file/