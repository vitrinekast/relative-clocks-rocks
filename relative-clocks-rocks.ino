#include <Adafruit_MCP4728.h>
#include <Encoder.h>
#include <Button.h>
#include <Wire.h>

#define ENCODER_DO_NOT_USE_INTERRUPTS

Adafruit_MCP4728 mcp;

const long gate_l = 40;
const int clockPin = A6;

uint16_t bpm = 200;
float divideQ = 1.0;
uint16_t divideQINT = 1;

bool clock_in_high;
unsigned long stepMs;
int32_t knob_position = 0;
unsigned long clocked_now = millis();

struct ChannelState {
    unsigned long next_time;
    unsigned long next_gated;
    bool state;
    float divider;
    channel channel;
    int id;
};

ChannelState channels_[4] = {
    {0,0,0,1.0,MCP4728_CHANNEL_A, 0},
    {0,0,0,1.0,MCP4728_CHANNEL_B, 1},
    {0,0,0,1.0,MCP4728_CHANNEL_C, 2},
    {0,0,0,1.0,MCP4728_CHANNEL_D, 3}
};


Encoder knob(2, 3);  // pins A, B
Button button(4);


void setup() {
  Serial.begin(9600);
  while (!Serial)
    delay(10);

  stepMs = (60000UL / bpm) * 4;

  pinMode(LED_BUILTIN, OUTPUT);

  unsigned long t = millis();

  button.begin();

  if (!mcp.begin()) {
    Serial.println("Failed to find MCP4728 chip");
    while (1) {
      delay(10);
    }
  }


  for(auto &ch : channels_) {
    mcp.setChannelValue(ch.channel, 0);
    ch.next_time = t + stepMs * ch.divider;
  }

  delay(50);
  
  for(auto &ch : channels_) {
    mcp.setChannelValue(ch.channel, 4095);
    delay(150);
    mcp.setChannelValue(ch.channel, 0);
  } 
}

void updateBPM(int p) {
  Serial.println("update bpm");
  bpm += p;
  stepMs = (60000UL / bpm) * 4;  
}


float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};


void updateDividers(int p) {
  Serial.print("update dividers - ");
  Serial.print(knob_position);
  Serial.print(" - ");
  divideQINT  += p;


  divideQ = mapfloat(knob_position, -64.0, 64.0, 0.0, 2.0);
  Serial.println(divideQ);
  
  // int idx = 1;

  // for(auto &ch : channels_) {
  //   float step = 0.0005 * idx * divideQ;
  //   idx++;
  //   ch.divider = 1 + step;
  //   // Serial.print(ch.channel);
  //   // Serial.print(": ");
  //   // Serial.println(ch.divider);
  // }
}

void loop() {
  unsigned long now = millis();

  digitalWrite(LED_BUILTIN, divideQ == 1.0 ? HIGH : LOW);

  clock_in_high = analogRead(clockPin) > 512;

  // when the CLOCK in is HIGH, re-calculate the next-time for the channels, and start timing the BPM.
  if(clock_in_high) {
    // if BPM has not been clocked before, set the clocked_now
    if(clocked_now < 10) {
      clocked_now = now;
    } else {
      unsigned long diff = now - clocked_now;
      bpm = (60000UL / diff) * 4;
      stepMs = diff;
      clocked_now = now;
    }

    for(auto &ch : channels_) {
      ch.next_time = now + stepMs * ch.divider;
    }
  }
  int32_t new_position = knob.read() / 4;

  // write to channels
  for(auto &ch : channels_) {

    // set channel state to 1 if passed the upcoming time.
    if(now >= ch.next_time) {
      ch.next_time += (ch.id == 0 ? stepMs : stepMs * pow(divideQ, ch.id));
      ch.state = true;
      mcp.setChannelValue(ch.channel, 4095);
      unsigned long end = now + gate_l;
      unsigned long max_end = ch.next_time - 50;
      if(end > max_end) end = max_end;
      ch.next_gated = end; 
    } else if (now >= ch.next_gated && ch.state) {
        mcp.setChannelValue(ch.channel, 0);
        ch.state = false; 
    }
  }

  // parse the encoder
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
