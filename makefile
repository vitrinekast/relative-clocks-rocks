# Name of your Arduino project (matches the folder)
TARGET = ../relative-clocks-rocks
BOARD_TAG = nano
PORT = /dev/cu.usbserial-210  # macOS usually /dev/cu.usbmodem*

all: build upload

build:
	arduino-cli compile --fqbn arduino:avr:$(BOARD_TAG) $(TARGET)

upload:
	arduino-cli upload -p $(PORT) --fqbn arduino:avr:$(BOARD_TAG) $(TARGET)

clean:
	rm -rf $(TARGET)/build
