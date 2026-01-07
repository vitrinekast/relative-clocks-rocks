# Relative Clocks (Rocks)
WIP Eurorack module that is a relative clock divider.
4 output channels all output the same clock, but each channel is just a little bit slower.

- 4-channel 12 bit output
- Speed adjustment internal clock
- External clock sync
- Adjust output wave 
- Reset button
- ALT button

## arduino cli
using the arduino cli to compile and upload the firmware. Info can be found [here](https://docs.arduino.cc/arduino-cli/getting-started/).


```SHELL
arduino-cli compile --fqbn arduino:avr:nano app.ino
arduino-cli upload -p /dev/cu.usbserial-210 --fqbn arduino:avr:nano ../relative-clocks-rocks
```