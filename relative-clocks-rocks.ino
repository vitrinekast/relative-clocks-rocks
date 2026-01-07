#include <Adafruit_MCP4728.h>
#include <Encoder.h>
#include <Button.h>
#include <Wire.h>

#define ENCODER_DO_NOT_USE_INTERRUPTS;

Adafruit_MCP4728 mcp;

int bpm = 120;
float dividers[4] = {1.0, 1.0, 1.0, 1.0};
static long gate_l = 40;
unsigned long nexts[4] = {0,0,0,0};
unsigned long nexts_end[4] = {0,0,0,0};
unsigned long stepMs;
int divideQ = 1;
unsigned int channel_state[4] = {0, 0, 0, 0};
channel channels[4] = {MCP4728_CHANNEL_A, MCP4728_CHANNEL_B, MCP4728_CHANNEL_C, MCP4728_CHANNEL_D};
const int clockPin = A6;
int clockValue;

int knob_position = 0;

Encoder knob(2, 3);  // pins A, B
Button button(4);

void setup() {
  Serial.begin(9600);
  while (!Serial)
    delay(10);

  stepMs = (60000UL / bpm) * 4;

  unsigned long t = millis();

  for (int i = 0; i < 4; i++) {
    nexts[i] = t + stepMs * dividers[i];
  }

  button.begin();

  if (!mcp.begin()) {
    Serial.println("Failed to find MCP4728 chip");
    while (1) {
      delay(10);
    }
  }

  for(int i = 0; i < 4; i++) {
    mcp.setChannelValue(channels[i], 0);
  } 
  
  delay(50);
  for(int i = 0; i < 4; i++) {
    mcp.setChannelValue(channels[i], 4095);
    delay(150);
    mcp.setChannelValue(channels[i], 0);
  } 
}

void updateBPM(int p) {
  Serial.println("update bpm");
  bpm += p;
  stepMs = (60000UL / bpm) * 4;  
}

void updateDividers(int p) {
  Serial.print("update dividers");
  divideQ += p;

  for (int i = 1; i < 4; i++) {
    float step = 0.0005 * i * divideQ;
    dividers[i] = 1 + step;
    Serial.print(i);
    Serial.print(": ");
    Serial.println(dividers[i]);
  }
}

void loop() {
  unsigned long now = millis();

  clockValue = analogRead(clockPin);

  Serial.println(clockValue);

  for(int i = 0; i < 4; i++) {
    if(now >= nexts[i]) {
      nexts[i] += stepMs * dividers[i];
      channel_state[i] = 1;
      mcp.setChannelValue(channels[i], 4095);
      unsigned long end = now + gate_l;

      unsigned long max_end = nexts[i] - 50;
      if(end > max_end) end = max_end;

      nexts_end[i] = end; 
    } else if (now >= nexts_end[i] && channel_state[i] == 1) {
        mcp.setChannelValue(channels[i], 0);
        channel_state[i] = 0; 
    }
  }

  long new_position = knob.read() / 4;
  
    if(new_position != knob_position) {
      long diff = new_position - knob_position;
      knob_position = new_position;
      if(button.read() == Button::PRESSED) {
        updateBPM(diff);
      } else {
        updateDividers(diff);
      }
    }
}
